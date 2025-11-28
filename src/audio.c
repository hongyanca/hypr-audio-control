#define _POSIX_C_SOURCE 200809L
#include "audio.h"
#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DEVICES 32
#define LINE_BUF_SIZE 1024

static void get_icon_name(const char *name, char *icon_name, size_t max_len) {
  char lower_name[256];
  strncpy(lower_name, name, sizeof(lower_name) - 1);
  lower_name[sizeof(lower_name) - 1] = '\0';

  // Convert to lowercase
  for (int i = 0; lower_name[i]; i++) {
    lower_name[i] = tolower(lower_name[i]);
  }

  if (strstr(lower_name, "headphone")) {
    strncpy(icon_name, "audio-headphones-symbolic", max_len);
  } else if (strstr(lower_name, "hdmi") || strstr(lower_name, "tv") ||
             strstr(lower_name, "display")) {
    strncpy(icon_name, "video-display-symbolic", max_len);
  } else if (strstr(lower_name, "bluetooth")) {
    strncpy(icon_name, "bluetooth-active-symbolic", max_len);
  } else {
    strncpy(icon_name, "audio-speakers-symbolic", max_len);
  }
}

AudioDevice *get_audio_devices(int *count) {
  FILE *fp;
  char line[LINE_BUF_SIZE];
  AudioDevice *devices = malloc(sizeof(AudioDevice) * (MAX_DEVICES + 1));
  *count = 0;

  if (!devices)
    return NULL;

  fp = popen("wpctl status", "r");
  if (fp == NULL) {
    free(devices);
    return NULL;
  }

  bool in_sinks = false;
  regex_t regex;
  // Regex: ^\s*│\s*(\*)?\s*(\d+)\.\s+(.+?)\s+\[vol:\s*([\d\.]+)
  // C string escaping makes this ugly
  const char *pattern =
      "^[[:space:]]*│[[:space:]]*(\\*?)[[:space:]]*([0-9]+)\\.[[:space:]]+(.+?)"
      "[[:space:]]+\\[vol:[[:space:]]*([0-9\\.]+)";

  if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
    pclose(fp);
    free(devices);
    return NULL;
  }

  while (fgets(line, sizeof(line), fp) != NULL) {
    if (strstr(line, "Sinks:")) {
      in_sinks = true;
      continue;
    }

    if (in_sinks) {
      // Check for end of section
      // If line doesn't start with │ (ignoring whitespace), it's likely end of
      // section But we need to be careful. The regex expects │. Let's check if
      // it contains "Sources:" or "Filters:" or "Streams:"
      if (strstr(line, "Sources:") || strstr(line, "Filters:") ||
          strstr(line, "Streams:")) {
        in_sinks = false;
        break;
      }

      // Simple check for empty line or line without pipe might be enough,
      // but let's rely on regex matching for valid lines.

      regmatch_t matches[5];
      if (regexec(&regex, line, 5, matches, 0) == 0) {
        if (*count >= MAX_DEVICES)
          break;

        AudioDevice *dev = &devices[*count];

        // Group 1: "*" or empty (Active)
        dev->is_active =
            (matches[1].rm_so != -1 && matches[1].rm_eo > matches[1].rm_so);

        // Group 2: ID
        char id_str[16];
        int len = matches[2].rm_eo - matches[2].rm_so;
        if (len >= (int)sizeof(id_str))
          len = sizeof(id_str) - 1;
        strncpy(id_str, line + matches[2].rm_so, len);
        id_str[len] = '\0';
        dev->id = atoi(id_str);

        // Group 3: Name
        len = matches[3].rm_eo - matches[3].rm_so;
        if (len >= (int)sizeof(dev->name))
          len = sizeof(dev->name) - 1;
        strncpy(dev->name, line + matches[3].rm_so, len);
        dev->name[len] = '\0';

        // Group 4: Volume
        char vol_str[16];
        len = matches[4].rm_eo - matches[4].rm_so;
        if (len >= (int)sizeof(vol_str))
          len = sizeof(vol_str) - 1;
        strncpy(vol_str, line + matches[4].rm_so, len);
        vol_str[len] = '\0';
        dev->volume = atof(vol_str);

        get_icon_name(dev->name, dev->icon_name, sizeof(dev->icon_name));

        (*count)++;
      }
    }
  }

  regfree(&regex);
  pclose(fp);

  // Terminator
  devices[*count].id = -1;
  return devices;
}

void set_volume(int device_id, double value) {
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "wpctl set-volume %d %.2f", device_id, value);
  system(cmd);
}

void set_default_device(int device_id) {
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "wpctl set-default %d", device_id);
  system(cmd);
}
