#include "audio.h"
#include <gtk/gtk.h>
#include <gtk4-layer-shell/gtk4-layer-shell.h>
#include <stdlib.h>
#include <string.h>

// Global state
static GtkApplication *app;
static GtkWidget *window;
static GtkWidget *main_box;
static GtkWidget *volume_scale;
static GtkWidget *output_box;
static GtkWidget *theme_button;
static bool is_dark_theme = false;
static int active_device_id = -1;
static GSettings *settings;

// Forward declarations
static void setup_ui(GtkWidget *win);
static void apply_theme();
static void on_device_clicked(GtkGestureClick *gesture, int n_press, double x,
                              double y, gpointer user_data);

static void quit_app() { g_application_quit(G_APPLICATION(app)); }

static void toggle_theme(GtkButton *btn, gpointer user_data) {
  (void)btn;
  (void)user_data;
  is_dark_theme = !is_dark_theme;
  apply_theme();
}

static void apply_theme() {
  if (is_dark_theme) {
    gtk_widget_add_css_class(main_box, "dark-theme");
    gtk_button_set_icon_name(GTK_BUTTON(theme_button),
                             "weather-clear-night-symbolic");
  } else {
    gtk_widget_remove_css_class(main_box, "dark-theme");
    gtk_button_set_icon_name(GTK_BUTTON(theme_button),
                             "weather-clear-symbolic");
  }
}

static void update_theme_from_system(GSettings *s, gchar *key,
                                     gpointer user_data) {
  (void)key;
  (void)user_data;
  char *scheme = g_settings_get_string(s, "color-scheme");
  is_dark_theme = (g_strcmp0(scheme, "prefer-dark") == 0);
  g_free(scheme);
  apply_theme();
}

static void on_volume_changed(GtkRange *range, gpointer user_data) {
  // user_data is intptr_t device_id
  int dev_id = (int)(intptr_t)user_data;
  double value = gtk_range_get_value(range) / 100.0;
  set_volume(dev_id, value);
}

static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval,
                               guint keycode, GdkModifierType state,
                               gpointer user_data) {
  (void)controller;
  (void)keycode;
  (void)user_data;
  const char *key_name = gdk_keyval_name(keyval);
  bool ctrl_pressed = (state & GDK_CONTROL_MASK);
  (void)ctrl_pressed; // Used in logic but maybe optimized out if not fully
                      // used? Actually it was unused in logic below.

  if (g_strcmp0(key_name, "q") == 0) {
    quit_app();
    return TRUE;
  }

  if (active_device_id != -1 && volume_scale) {
    if (g_strcmp0(key_name, "grave") == 0) {
      gtk_range_set_value(GTK_RANGE(volume_scale), 0);
      return TRUE;
    }
    if (g_strcmp0(key_name, "minus") == 0) {
      double val = gtk_range_get_value(GTK_RANGE(volume_scale));
      gtk_range_set_value(GTK_RANGE(volume_scale), val - 5);
      return TRUE;
    }
    if (g_strcmp0(key_name, "equal") == 0) {
      double val = gtk_range_get_value(GTK_RANGE(volume_scale));
      gtk_range_set_value(GTK_RANGE(volume_scale), val + 5);
      return TRUE;
    }
    if (keyval >= GDK_KEY_0 && keyval <= GDK_KEY_9) {
      int num = keyval - GDK_KEY_0;
      if (num == 0)
        num = 10;
      gtk_range_set_value(GTK_RANGE(volume_scale), num * 10);
      return TRUE;
    }
  }

  return FALSE;
}

static void launch_program(const char *cmd) {
  // Simple fork/exec or system
  // system() blocks, so we should use g_spawn_command_line_async
  GError *error = NULL;
  g_spawn_command_line_async(cmd, &error);
  if (error) {
    g_printerr("Failed to launch %s: %s\n", cmd, error->message);
    g_error_free(error);
  }
}

static void on_launcher_clicked(GtkGestureClick *gesture, int n_press, double x,
                                double y, gpointer user_data) {
  (void)gesture;
  (void)n_press;
  (void)x;
  (void)y;
  const char *cmd = (const char *)user_data;
  launch_program(cmd);
}

static GtkWidget *create_device_row(AudioDevice *dev) {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_add_css_class(box, "device-row");
  if (dev->is_active) {
    gtk_widget_add_css_class(box, "device-row-active");
  }

  GtkGesture *gesture = gtk_gesture_click_new();
  g_signal_connect(gesture, "pressed", G_CALLBACK(on_device_clicked),
                   (gpointer)(intptr_t)dev->id);
  gtk_widget_add_controller(box, GTK_EVENT_CONTROLLER(gesture));

  GtkWidget *icon_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_add_css_class(icon_box, "device-icon-box");
  if (dev->is_active) {
    gtk_widget_add_css_class(icon_box, "device-icon-box-active");
  }

  GtkWidget *icon = gtk_image_new_from_icon_name(dev->icon_name);
  gtk_widget_add_css_class(icon, "device-icon");
  gtk_box_append(GTK_BOX(icon_box), icon);
  gtk_box_append(GTK_BOX(box), icon_box);

  GtkWidget *label = gtk_label_new(dev->name);
  gtk_widget_add_css_class(label, "device-label");
  gtk_box_append(GTK_BOX(box), label);

  return box;
}

static void on_device_clicked(GtkGestureClick *gesture, int n_press, double x,
                              double y, gpointer user_data) {
  (void)gesture;
  (void)n_press;
  (void)x;
  (void)y;
  int id = (int)(intptr_t)user_data;
  set_default_device(id);
  setup_ui(window); // Refresh UI
}

static void add_launcher_label(GtkWidget *parent, const char *text,
                               const char *cmd, bool enabled) {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_add_css_class(box, "launcher-row");

  GtkWidget *icon = gtk_image_new_from_icon_name("preferences-system");
  gtk_image_set_pixel_size(GTK_IMAGE(icon), 30);
  gtk_widget_add_css_class(icon, "launcher-icon");
  gtk_box_append(GTK_BOX(box), icon);

  GtkWidget *label = gtk_label_new(text);
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  gtk_widget_add_css_class(label, "launcher-label");
  gtk_box_append(GTK_BOX(box), label);

  if (enabled) {
    GtkGesture *gesture = gtk_gesture_click_new();
    g_signal_connect(gesture, "pressed", G_CALLBACK(on_launcher_clicked),
                     (gpointer)cmd);
    gtk_widget_add_controller(box, GTK_EVENT_CONTROLLER(gesture));
    gtk_widget_set_cursor_from_name(box, "pointer");
  } else {
    gtk_widget_add_css_class(box, "launcher-row-disabled");
    gtk_widget_set_cursor_from_name(box, "default");
  }

  gtk_box_append(GTK_BOX(parent), box);
}

static void setup_ui(GtkWidget *win) {
  // Clear existing children
  GtkWidget *child = gtk_window_get_child(GTK_WINDOW(win));
  if (child) {
    gtk_window_set_child(GTK_WINDOW(win), NULL);
  }

  main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(main_box, "main-window");
  gtk_widget_set_size_request(main_box, 400, -1);
  gtk_window_set_child(GTK_WINDOW(win), main_box);

  // --- Sound Section ---
  GtkWidget *sound_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_add_css_class(sound_box, "section-box");
  gtk_box_append(GTK_BOX(main_box), sound_box);

  // Header
  GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_add_css_class(header_box, "header-box");

  GtkWidget *sound_label = gtk_label_new("Sound");
  gtk_widget_set_halign(sound_label, GTK_ALIGN_START);
  gtk_widget_add_css_class(sound_label, "section-title");
  gtk_widget_set_hexpand(sound_label, TRUE);
  gtk_box_append(GTK_BOX(header_box), sound_label);

  theme_button = gtk_button_new();
  gtk_widget_add_css_class(theme_button, "theme-button");
  g_signal_connect(theme_button, "clicked", G_CALLBACK(toggle_theme), NULL);
  gtk_box_append(GTK_BOX(header_box), theme_button);

  GtkWidget *close_button = gtk_button_new_with_label("✕");
  gtk_widget_add_css_class(close_button, "close-button");
  g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(quit_app), NULL);
  gtk_box_append(GTK_BOX(header_box), close_button);

  gtk_box_append(GTK_BOX(sound_box), header_box);

  // Volume Slider
  GtkWidget *volume_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_add_css_class(volume_box, "volume-box");

  volume_scale =
      gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  gtk_widget_set_hexpand(volume_scale, TRUE);
  gtk_widget_add_css_class(volume_scale, "volume-slider");
  gtk_box_append(GTK_BOX(volume_box), volume_scale);
  gtk_box_append(GTK_BOX(sound_box), volume_box);

  // Separator
  GtkWidget *sep1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_add_css_class(sep1, "separator");
  gtk_box_append(GTK_BOX(main_box), sep1);

  // --- Output Section ---
  output_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_add_css_class(output_box, "section-box");
  gtk_box_append(GTK_BOX(main_box), output_box);

  GtkWidget *output_label = gtk_label_new("Output");
  gtk_widget_set_halign(output_label, GTK_ALIGN_START);
  gtk_widget_add_css_class(output_label, "section-title");
  gtk_box_append(GTK_BOX(output_box), output_label);

  // Device List
  int count;
  AudioDevice *devices = get_audio_devices(&count);
  double current_volume = 0.5;
  active_device_id = -1;

  if (count == 0) {
    // No devices
    // TODO: Handle no devices
  } else {
    for (int i = 0; i < count; i++) {
      GtkWidget *row = create_device_row(&devices[i]);
      gtk_box_append(GTK_BOX(output_box), row);
      if (devices[i].is_active) {
        current_volume = devices[i].volume;
        active_device_id = devices[i].id;
      }
    }
  }
  free(devices);

  gtk_range_set_value(GTK_RANGE(volume_scale), current_volume * 100);
  if (active_device_id != -1) {
    g_signal_connect(volume_scale, "value-changed",
                     G_CALLBACK(on_volume_changed),
                     (gpointer)(intptr_t)active_device_id);
  }

  // Separator
  GtkWidget *sep2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_add_css_class(sep2, "separator");
  gtk_box_append(GTK_BOX(main_box), sep2);

  // --- Footer ---
  GtkWidget *footer_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_add_css_class(footer_box, "footer-box");

  GtkWidget *pref_label = gtk_label_new("Advanced Controls");
  gtk_widget_set_halign(pref_label, GTK_ALIGN_START);
  gtk_widget_add_css_class(pref_label, "footer-label");
  gtk_box_append(GTK_BOX(footer_box), pref_label);

  // Check for wiremix and pavucontrol
  bool has_wiremix = (system("which wiremix > /dev/null 2>&1") == 0);
  bool has_omarchy =
      (system("which omarchy-launch-or-focus-tui > /dev/null 2>&1") == 0);
  bool has_pavu = (system("which pavucontrol > /dev/null 2>&1") == 0);

  const char *wiremix_cmd =
      has_omarchy ? "omarchy-launch-or-focus-tui wiremix" : "wiremix";
  add_launcher_label(footer_box, "wiremix", wiremix_cmd, has_wiremix);
  add_launcher_label(footer_box, "PulseAudio Volume Control", "pavucontrol",
                     has_pavu);

  gtk_box_append(GTK_BOX(main_box), footer_box);

  apply_theme();
}

static void activate(GtkApplication *app, gpointer user_data) {
  (void)user_data;
  window = gtk_application_window_new(app);

  // Layer Shell
  gtk_layer_init_for_window(GTK_WINDOW(window));
  gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_TOP);
  gtk_layer_set_keyboard_mode(GTK_WINDOW(window),
                              GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND);
  gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
  gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

  // Keyboard Controller
  GtkEventController *key_controller = gtk_event_controller_key_new();
  g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed),
                   NULL);
  gtk_widget_add_controller(window, key_controller);

  // CSS
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(provider, "style/style.css");
  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);

  setup_ui(window);

  // System Theme
  settings = g_settings_new("org.gnome.desktop.interface");
  g_signal_connect(settings, "changed::color-scheme",
                   G_CALLBACK(update_theme_from_system), NULL);
  update_theme_from_system(settings, NULL, NULL);

  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
  app = gtk_application_new("ca.hongyan.audiocontrol",
                            G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
