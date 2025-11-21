import gi
gi.require_version('Gtk', '4.0')
gi.require_version('Gtk4LayerShell', '1.0')
from gi.repository import Gtk, Gdk, Gtk4LayerShell
from .audio import AudioManager

class AudioControlApp(Gtk.Application):
    def __init__(self):
        super().__init__(application_id='com.example.audiocontrol')

    def do_activate(self):
        self.window = Gtk.ApplicationWindow(application=self)
        self.setup_layer_shell(self.window)
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
        Gtk4LayerShell.set_margin(window, Gtk4LayerShell.Edge.TOP, 10)
        Gtk4LayerShell.set_margin(window, Gtk4LayerShell.Edge.RIGHT, 10)

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
        window.set_child(main_box)

        # --- Sound Section ---
        sound_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        sound_box.add_css_class('section-box')
        main_box.append(sound_box)

        sound_label = Gtk.Label(label="Sound", xalign=0)
        sound_label.add_css_class('section-title')
        sound_box.append(sound_label)

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

        # Separator
        sep2 = Gtk.Separator(orientation=Gtk.Orientation.HORIZONTAL)
        sep2.add_css_class('separator')
        main_box.append(sep2)

        # --- Footer ---
        footer_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=10)
        footer_box.add_css_class('footer-box')
        
        pref_label = Gtk.Label(label="Sound Preferences...", xalign=0)
        pref_label.add_css_class('footer-label')
        footer_box.append(pref_label)
        
        main_box.append(footer_box)

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
