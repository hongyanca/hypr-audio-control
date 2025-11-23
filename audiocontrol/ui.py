import gi
gi.require_version('Gtk', '4.0')
gi.require_version('Gtk4LayerShell', '1.0')
from gi.repository import Gtk, Gdk, Gtk4LayerShell
from .audio import AudioManager
import shutil
import subprocess

class AudioControlApp(Gtk.Application):
    def __init__(self):
        super().__init__(application_id='ca.hongyan.audiocontrol')

    def do_activate(self):
        self.window = Gtk.ApplicationWindow(application=self)
        
        # Initialize layer shell first
        self.setup_layer_shell(self.window)
        
        # Disable resizability after layer shell init
        self.window.set_resizable(False)
        
        # Setup keyboard event controller
        self.setup_keyboard_controller(self.window)
        
        self.setup_ui(self.window)
        self.window.present()

    def setup_layer_shell(self, window):
        Gtk4LayerShell.init_for_window(window)
        Gtk4LayerShell.set_layer(window, Gtk4LayerShell.Layer.TOP)
        Gtk4LayerShell.set_keyboard_mode(window, Gtk4LayerShell.KeyboardMode.ON_DEMAND)

        # Anchor to top right
        Gtk4LayerShell.set_anchor(window, Gtk4LayerShell.Edge.TOP, True)
        Gtk4LayerShell.set_anchor(window, Gtk4LayerShell.Edge.RIGHT, True)
        
        # Margins
        # Gtk4LayerShell.set_margin(window, Gtk4LayerShell.Edge.TOP, 100)
        # Gtk4LayerShell.set_margin(window, Gtk4LayerShell.Edge.RIGHT, 10)

    def setup_keyboard_controller(self, window):
        """Setup keyboard event controller for shortcuts"""
        key_controller = Gtk.EventControllerKey.new()
        key_controller.connect('key-pressed', self.on_key_pressed)
        window.add_controller(key_controller)
        
        # Store reference to active device and scale for keyboard control
        self.active_device_id = None
        self.volume_scale = None

    def on_key_pressed(self, controller, keyval, keycode, state):
        """Handle keyboard shortcuts"""
        # Get key name
        key_name = Gdk.keyval_name(keyval)
        
        # Check for Ctrl modifier
        ctrl_pressed = state & Gdk.ModifierType.CONTROL_MASK
        
        # Exit on 'q' or 'Ctrl+q'
        if key_name == 'q' or (key_name == 'q' and ctrl_pressed):
            self.quit()
            return True
        
        # Volume control with number keys 1-9, 0
        # 1 = 10%, 2 = 20%, ..., 9 = 90%, 0 = 100%
        volume_map = {
            '1': 10, '2': 20, '3': 30, '4': 40, '5': 50,
            '6': 60, '7': 70, '8': 80, '9': 90, '0': 100
        }
        
        if key_name in volume_map and self.active_device_id and self.volume_scale:
            volume_percent = volume_map[key_name]
            self.volume_scale.set_value(volume_percent)
            return True

        # Set volume to 0% with '`' (grave)
        if key_name == 'grave' and self.active_device_id and self.volume_scale:
            self.volume_scale.set_value(0)
            return True
            
        # Decrease volume by 5% with '-'
        if key_name == 'minus' and self.active_device_id and self.volume_scale:
            current_value = self.volume_scale.get_value()
            new_value = max(0, current_value - 5)
            self.volume_scale.set_value(new_value)
            return True

        # Increase volume by 5% with '='
        if key_name == 'equal' and self.active_device_id and self.volume_scale:
            current_value = self.volume_scale.get_value()
            new_value = min(100, current_value + 5)
            self.volume_scale.set_value(new_value)
            return True
        
        return False

    def setup_ui(self, window=None):
        if window is None:
            window = self.window
            
        # Clear existing content if refreshing
        old_child = window.get_child()
        if old_child:
            window.set_child(None)
        # Load CSS
        css_provider = Gtk.CssProvider()
        css_provider.load_from_path('style.css')
        Gtk.StyleContext.add_provider_for_display(
            Gdk.Display.get_default(),
            css_provider,
            Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )

        main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        main_box.add_css_class('main-window')
        
        # Set fixed size for the main content
        main_box.set_size_request(400, -1)  # Width: 400px, Height: auto
        
        window.set_child(main_box)

        # --- Sound Section ---
        sound_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        sound_box.add_css_class('section-box')
        main_box.append(sound_box)

        # Header with Sound title and Close button on the same line
        header_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        header_box.add_css_class('header-box')
        
        sound_label = Gtk.Label(label="Sound", xalign=0)
        sound_label.add_css_class('section-title')
        sound_label.set_hexpand(True)
        header_box.append(sound_label)
        
        # Close button
        close_button = Gtk.Button()
        close_button.set_label("✕")
        close_button.add_css_class('close-button')
        close_button.connect('clicked', lambda btn: self.quit())
        header_box.append(close_button)
        
        sound_box.append(header_box)


        # Volume Slider Container
        volume_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=10)
        volume_box.add_css_class('volume-box')
        
        scale = Gtk.Scale.new_with_range(Gtk.Orientation.HORIZONTAL, 0, 100, 1)
        scale.set_hexpand(True)
        scale.add_css_class('volume-slider')
        
        volume_box.append(scale)
        sound_box.append(volume_box)

        # Separator
        sep1 = Gtk.Separator(orientation=Gtk.Orientation.HORIZONTAL)
        sep1.add_css_class('separator')
        main_box.append(sep1)

        # --- Output Section ---
        output_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=5)
        output_box.add_css_class('section-box')
        main_box.append(output_box)

        output_label = Gtk.Label(label="Output", xalign=0)
        output_label.add_css_class('section-title')
        output_box.append(output_label)

        # Device List
        devices = AudioManager.get_audio_devices()
        if not devices:
            devices = [("No devices found", "audio-speakers-symbolic", False, 0.0, None)]

        current_volume = 0.5 # Default
        active_device_id = None
        
        for name, icon_name, active, volume, dev_id in devices:
            row = self.create_device_row(name, icon_name, active, dev_id)
            output_box.append(row)
            if active:
                current_volume = volume
                active_device_id = dev_id

        scale.set_value(current_volume * 100)
        if active_device_id:
            scale.connect('value-changed', self.on_volume_changed, active_device_id)
        
        # Store references for keyboard control
        self.volume_scale = scale
        self.active_device_id = active_device_id

        # Separator
        sep2 = Gtk.Separator(orientation=Gtk.Orientation.HORIZONTAL)
        sep2.add_css_class('separator')
        main_box.append(sep2)

        # --- Footer ---
        footer_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
        footer_box.add_css_class('footer-box')
        
        pref_label = Gtk.Label(label="Advanced Controls", xalign=0)
        pref_label.add_css_class('footer-label')
        footer_box.append(pref_label)

        # Wiremix Launcher
        wiremix_path = shutil.which('wiremix')
        omarchy_launcher = shutil.which('omarchy-launch-or-focus-tui')
        
        def launch_wiremix(gesture, n_press, x, y):
            cmd = []
            if omarchy_launcher:
                cmd = [omarchy_launcher, 'wiremix']
            else:
                cmd = ['wiremix']
            subprocess.Popen(cmd)

        self.add_launcher_label(footer_box, "wiremix", wiremix_path is not None, launch_wiremix)

        # Pavucontrol Launcher
        pavu_path = shutil.which('pavucontrol')
        
        def launch_pavu(gesture, n_press, x, y):
            subprocess.Popen(['pavucontrol'])

        self.add_launcher_label(footer_box, "PulseAudio Volume Control", pavu_path is not None, launch_pavu)
        
        main_box.append(footer_box)

    def add_launcher_label(self, parent_box, text, enabled, callback):
        box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=12)
        box.add_css_class('launcher-row')
        
        # Icon
        icon = Gtk.Image.new_from_icon_name("preferences-system")
        icon.set_pixel_size(30)
        icon.add_css_class('launcher-icon')
        box.append(icon)

        # Label
        label = Gtk.Label(label=text, xalign=0)
        label.add_css_class('launcher-label')
        box.append(label)
        
        if enabled:
            gesture = Gtk.GestureClick.new()
            gesture.connect('pressed', callback)
            box.add_controller(gesture)
            box.set_cursor(Gdk.Cursor.new_from_name("pointer", None))
        else:
            box.add_css_class('launcher-row-disabled')
            box.set_cursor(Gdk.Cursor.new_from_name("default", None))
            
        parent_box.append(box)

    def create_device_row(self, name, icon_name, active, device_id):
        box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=12)
        box.add_css_class('device-row')
        if active:
            box.add_css_class('device-row-active')

        # Make the row clickable
        gesture = Gtk.GestureClick.new()
        gesture.connect('pressed', lambda g, n, x, y: self.on_device_clicked(device_id))
        box.add_controller(gesture)

        # Icon circle background
        icon_box = Gtk.Box()
        icon_box.add_css_class('device-icon-box')
        if active:
            icon_box.add_css_class('device-icon-box-active')
        
        icon = Gtk.Image.new_from_icon_name(icon_name)
        icon.add_css_class('device-icon')
        # icon.set_halign(Gtk.Align.CENTER)
        # icon.set_valign(Gtk.Align.CENTER)
        icon_box.append(icon)
        
        box.append(icon_box)

        label = Gtk.Label(label=name)
        label.add_css_class('device-label')
        box.append(label)

        return box

    def on_volume_changed(self, scale, device_id):
        value = scale.get_value() / 100.0
        AudioManager.set_volume(device_id, value)

    def on_device_clicked(self, device_id):
        AudioManager.set_default_device(device_id)
        # Refresh the UI to reflect the change
        self.setup_ui()
