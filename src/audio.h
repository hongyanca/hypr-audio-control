#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

typedef struct {
    char name[256];
    char icon_name[64];
    bool is_active;
    double volume;
    int id;
} AudioDevice;

// Returns an array of AudioDevice, terminated by a device with id -1.
// The caller must free the returned array.
AudioDevice* get_audio_devices(int *count);

void set_volume(int device_id, double value);
void set_default_device(int device_id);

#endif
