/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2007 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "alMain.h"
#include "alu.h"
#include "threads.h"
#include "compat.h"

#include "backends/base.h"

#include <jack/jack.h>
#include <jack/ringbuffer.h>


static const ALCchar jackDevice[] = "JACK Default";


#ifdef HAVE_DYNLOAD
#define JACK_FUNCS(MAGIC)          \
    MAGIC(jack_client_open);       \
    MAGIC(jack_client_close);      \
    MAGIC(jack_client_name_size);  \
    MAGIC(jack_get_client_name);   \
    MAGIC(jack_connect);           \
    MAGIC(jack_activate);          \
    MAGIC(jack_deactivate);        \
    MAGIC(jack_port_register);     \
    MAGIC(jack_port_unregister);   \
    MAGIC(jack_port_get_buffer);   \
    MAGIC(jack_port_name);         \
    MAGIC(jack_get_ports);         \
    MAGIC(jack_free);              \
    MAGIC(jack_get_sample_rate);   \
    MAGIC(jack_set_error_function); \
    MAGIC(jack_set_process_callback); \
    MAGIC(jack_set_buffer_size_callback); \
    MAGIC(jack_set_buffer_size);   \
    MAGIC(jack_get_buffer_size);

static void *jack_handle;
#define MAKE_FUNC(f) static __typeof(f) * p##f
JACK_FUNCS(MAKE_FUNC);
#undef MAKE_FUNC

#define jack_client_open pjack_client_open
#define jack_client_close pjack_client_close
#define jack_client_name_size pjack_client_name_size
#define jack_get_client_name pjack_get_client_name
#define jack_connect pjack_connect
#define jack_activate pjack_activate
#define jack_deactivate pjack_deactivate
#define jack_port_register pjack_port_register
#define jack_port_unregister pjack_port_unregister
#define jack_port_get_buffer pjack_port_get_buffer
#define jack_port_name pjack_port_name
#define jack_get_ports pjack_get_ports
#define jack_free pjack_free
#define jack_get_sample_rate pjack_get_sample_rate
#define jack_set_error_function pjack_set_error_function
#define jack_set_process_callback pjack_set_process_callback
#define jack_set_buffer_size_callback pjack_set_buffer_size_callback
#define jack_set_buffer_size pjack_set_buffer_size
#define jack_get_buffer_size pjack_get_buffer_size
#endif


static jack_options_t ClientOptions = JackNullOption;

static ALCboolean jack_load(void)
{
    ALCboolean error = ALC_FALSE;

#ifdef HAVE_DYNLOAD
    if(!jack_handle)
    {
#ifdef _WIN32
#define JACKLIB "libjack.dll"
#else
#define JACKLIB "libjack.so.0"
#endif
        jack_handle = LoadLib(JACKLIB);
        if(!jack_handle)
            return ALC_FALSE;

        error = ALC_FALSE;
#define LOAD_FUNC(f) do {                                                     \
    p##f = GetSymbol(jack_handle, #f);                                        \
    if(p##f == NULL) {                                                        \
        error = ALC_TRUE;                                                     \
    }                                                                         \
} while(0)
        JACK_FUNCS(LOAD_FUNC);
#undef LOAD_FUNC

        if(error)
        {
            CloseLib(jack_handle);
            jack_handle = NULL;
            return ALC_FALSE;
        }
    }
#endif

    return !error;
}


typedef struct ALCjackPlayback {
    DERIVE_FROM_TYPE(ALCbackend);

    jack_client_t *Client;
    jack_port_t *Port[MAX_OUTPUT_CHANNELS];

    ll_ringbuffer_t *Ring;
    alcnd_t Cond;

    volatile int killNow;
    althrd_t thread;
} ALCjackPlayback;

static int ALCjackPlayback_bufferSizeNotify(jack_nframes_t numframes, void *arg);

static int ALCjackPlayback_process(jack_nframes_t numframes, void *arg);
static int ALCjackPlayback_mixerProc(void *arg);

static void ALCjackPlayback_Construct(ALCjackPlayback *self, ALCdevice *device);
static void ALCjackPlayback_Destruct(ALCjackPlayback *self);
static ALCenum ALCjackPlayback_open(ALCjackPlayback *self, const ALCchar *name);
static void ALCjackPlayback_close(ALCjackPlayback *self);
static ALCboolean ALCjackPlayback_reset(ALCjackPlayback *self);
static ALCboolean ALCjackPlayback_start(ALCjackPlayback *self);
static void ALCjackPlayback_stop(ALCjackPlayback *self);
static DECLARE_FORWARD2(ALCjackPlayback, ALCbackend, ALCenum, captureSamples, void*, ALCuint)
static DECLARE_FORWARD(ALCjackPlayback, ALCbackend, ALCuint, availableSamples)
static ClockLatency ALCjackPlayback_getClockLatency(ALCjackPlayback *self);
static void ALCjackPlayback_lock(ALCjackPlayback *self);
static void ALCjackPlayback_unlock(ALCjackPlayback *self);
DECLARE_DEFAULT_ALLOCATORS(ALCjackPlayback)

DEFINE_ALCBACKEND_VTABLE(ALCjackPlayback);


static void ALCjackPlayback_Construct(ALCjackPlayback *self, ALCdevice *device)
{
    ALuint i;

    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCjackPlayback, ALCbackend, self);

    alcnd_init(&self->Cond);

    self->Client = NULL;
    for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
        self->Port[i] = NULL;
    self->Ring = NULL;

    self->killNow = 1;
}

static void ALCjackPlayback_Destruct(ALCjackPlayback *self)
{
    ALuint i;

    if(self->Client)
    {
        for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
        {
            if(self->Port[i])
                jack_port_unregister(self->Client, self->Port[i]);
            self->Port[i] = NULL;
        }
        jack_client_close(self->Client);
        self->Client = NULL;
    }

    alcnd_destroy(&self->Cond);

    ALCbackend_Destruct(STATIC_CAST(ALCbackend, self));
}


static int ALCjackPlayback_bufferSizeNotify(jack_nframes_t numframes, void *arg)
{
    ALCjackPlayback *self = arg;
    ALCdevice *device = STATIC_CAST(ALCbackend,self)->mDevice;
    ALuint bufsize;

    ALCjackPlayback_lock(self);
    device->UpdateSize = numframes;
    device->NumUpdates = 2;
    TRACE("%u update size x%u\n", device->UpdateSize, device->NumUpdates);

    bufsize = device->UpdateSize;
    if(ConfigValueUInt(al_string_get_cstr(device->DeviceName), "jack", "buffer-size", &bufsize))
        bufsize = maxu(NextPowerOf2(bufsize), device->UpdateSize);
    bufsize += device->UpdateSize;

    ll_ringbuffer_free(self->Ring);
    self->Ring = ll_ringbuffer_create(bufsize, FrameSizeFromDevFmt(device->FmtChans, device->FmtType));
    if(!self->Ring)
    {
        ERR("Failed to reallocate ringbuffer\n");
        aluHandleDisconnect(device);
    }
    ALCjackPlayback_unlock(self);
    return 0;
}


static int ALCjackPlayback_process(jack_nframes_t numframes, void *arg)
{
    ALCjackPlayback *self = arg;
    jack_default_audio_sample_t *out[MAX_OUTPUT_CHANNELS];
    ll_ringbuffer_data_t data[2];
    jack_nframes_t total = 0;
    jack_nframes_t todo;
    ALuint i, c, numchans;

    ll_ringbuffer_get_read_vector(self->Ring, data);

    for(c = 0;c < MAX_OUTPUT_CHANNELS && self->Port[c];c++)
        out[c] = jack_port_get_buffer(self->Port[c], numframes);
    numchans = c;

    todo = minu(numframes, data[0].len);
    for(c = 0;c < numchans;c++)
    {
        for(i = 0;i < todo;i++)
            out[c][i] = ((ALfloat*)data[0].buf)[i*numchans + c];
        out[c] += todo;
    }
    total += todo;

    todo = minu(numframes-total, data[1].len);
    if(todo > 0)
    {
        for(c = 0;c < numchans;c++)
        {
            for(i = 0;i < todo;i++)
                out[c][i] = ((ALfloat*)data[1].buf)[i*numchans + c];
            out[c] += todo;
        }
        total += todo;
    }

    ll_ringbuffer_read_advance(self->Ring, total);
    alcnd_signal(&self->Cond);

    if(numframes > total)
    {
        todo = numframes-total;
        for(c = 0;c < numchans;c++)
        {
            for(i = 0;i < todo;i++)
                out[c][i] = 0.0f;
        }
    }

    return 0;
}

static int ALCjackPlayback_mixerProc(void *arg)
{
    ALCjackPlayback *self = arg;
    ALCdevice *device = STATIC_CAST(ALCbackend,self)->mDevice;
    ll_ringbuffer_data_t data[2];

    SetRTPriority();
    althrd_setname(althrd_current(), MIXER_THREAD_NAME);

    ALCjackPlayback_lock(self);
    while(!self->killNow && device->Connected)
    {
        ALuint todo, len1, len2;

        /* NOTE: Unfortunately, there is an unavoidable race condition here.
         * It's possible for the process() method to run, updating the read
         * pointer and signaling the condition variable, in between the mixer
         * loop checking the write size and waiting for the condition variable.
         * This will cause the mixer loop to wait until the *next* process()
         * invocation, most likely writing silence for it.
         *
         * However, this should only happen if the mixer is running behind
         * anyway (as ideally we'll be asleep in alcnd_wait by the time the
         * process() method is invoked), so this behavior is not unwarranted.
         * It's unfortunate since it'll be wasting time sleeping that could be
         * used to catch up, but there's no way around it without blocking in
         * the process() method.
         */
        if(ll_ringbuffer_write_space(self->Ring) < device->UpdateSize)
        {
            alcnd_wait(&self->Cond, &STATIC_CAST(ALCbackend,self)->mMutex);
            continue;
        }

        ll_ringbuffer_get_write_vector(self->Ring, data);
        todo  = data[0].len + data[1].len;
        todo -= todo%device->UpdateSize;

        len1 = minu(data[0].len, todo);
        len2 = minu(data[1].len, todo-len1);

        aluMixData(device, data[0].buf, len1);
        if(len2 > 0)
            aluMixData(device, data[1].buf, len2);
        ll_ringbuffer_write_advance(self->Ring, todo);
    }
    ALCjackPlayback_unlock(self);

    return 0;
}


static ALCenum ALCjackPlayback_open(ALCjackPlayback *self, const ALCchar *name)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    const char *client_name = "alsoft";
    jack_status_t status;

    if(!name)
        name = jackDevice;
    else if(strcmp(name, jackDevice) != 0)
        return ALC_INVALID_VALUE;

    self->Client = jack_client_open(client_name, ClientOptions, &status, NULL);
    if(self->Client == NULL)
    {
        ERR("jack_client_open() failed, status = 0x%02x\n", status);
        return ALC_INVALID_VALUE;
    }
    if((status&JackServerStarted))
        TRACE("JACK server started\n");
    if((status&JackNameNotUnique))
    {
        client_name = jack_get_client_name(self->Client);
        TRACE("Client name not unique, got `%s' instead\n", client_name);
    }

    jack_set_process_callback(self->Client, ALCjackPlayback_process, self);
    jack_set_buffer_size_callback(self->Client, ALCjackPlayback_bufferSizeNotify, self);

    al_string_copy_cstr(&device->DeviceName, name);

    return ALC_NO_ERROR;
}

static void ALCjackPlayback_close(ALCjackPlayback *self)
{
    ALuint i;

    for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
    {
        if(self->Port[i])
            jack_port_unregister(self->Client, self->Port[i]);
        self->Port[i] = NULL;
    }
    jack_client_close(self->Client);
    self->Client = NULL;
}

static ALCboolean ALCjackPlayback_reset(ALCjackPlayback *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    ALuint numchans, i;
    ALuint bufsize;

    for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
    {
        if(self->Port[i])
            jack_port_unregister(self->Client, self->Port[i]);
        self->Port[i] = NULL;
    }

    /* Ignore the requested buffer metrics and just keep one JACK-sized buffer
     * ready for when requested. Note that one period's worth of audio in the
     * ring buffer will always be left unfilled because one element of the ring
     * buffer will not be writeable, and we only write in period-sized chunks.
     */
    device->Frequency = jack_get_sample_rate(self->Client);
    device->UpdateSize = jack_get_buffer_size(self->Client);
    device->NumUpdates = 2;

    bufsize = device->UpdateSize;
    if(ConfigValueUInt(al_string_get_cstr(device->DeviceName), "jack", "buffer-size", &bufsize))
        bufsize = maxu(NextPowerOf2(bufsize), device->UpdateSize);
    bufsize += device->UpdateSize;

    /* Force 32-bit float output. */
    device->FmtType = DevFmtFloat;

    numchans = ChannelsFromDevFmt(device->FmtChans);
    for(i = 0;i < numchans;i++)
    {
        char name[64];
        snprintf(name, sizeof(name), "channel_%d", i+1);
        self->Port[i] = jack_port_register(self->Client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        if(self->Port[i] == NULL)
        {
            ERR("Not enough JACK ports available for %s output\n", DevFmtChannelsString(device->FmtChans));
            if(i == 0) return ALC_FALSE;
            break;
        }
    }
    if(i < numchans)
    {
        if(i == 1)
            device->FmtChans = DevFmtMono;
        else
        {
            for(--i;i >= 2;i--)
            {
                jack_port_unregister(self->Client, self->Port[i]);
                self->Port[i] = NULL;
            }
            device->FmtChans = DevFmtStereo;
        }
    }

    ll_ringbuffer_free(self->Ring);
    self->Ring = ll_ringbuffer_create(bufsize, FrameSizeFromDevFmt(device->FmtChans, device->FmtType));
    if(!self->Ring)
    {
        ERR("Failed to allocate ringbuffer\n");
        return ALC_FALSE;
    }

    SetDefaultChannelOrder(device);

    return ALC_TRUE;
}

static ALCboolean ALCjackPlayback_start(ALCjackPlayback *self)
{
    const char **ports;
    ALuint i;

    if(jack_activate(self->Client))
    {
        ERR("Failed to activate client\n");
        return ALC_FALSE;
    }

    ports = jack_get_ports(self->Client, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
    if(ports == NULL)
    {
        ERR("No physical playback ports found\n");
        jack_deactivate(self->Client);
        return ALC_FALSE;
    }
    for(i = 0;i < MAX_OUTPUT_CHANNELS && self->Port[i];i++)
    {
        if(!ports[i])
        {
            ERR("No physical playback port for \"%s\"\n", jack_port_name(self->Port[i]));
            break;
        }
        if(jack_connect(self->Client, jack_port_name(self->Port[i]), ports[i]))
            ERR("Failed to connect output port \"%s\" to \"%s\"\n", jack_port_name(self->Port[i]), ports[i]);
    }
    jack_free(ports);

    self->killNow = 0;
    if(althrd_create(&self->thread, ALCjackPlayback_mixerProc, self) != althrd_success)
    {
        jack_deactivate(self->Client);
        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static void ALCjackPlayback_stop(ALCjackPlayback *self)
{
    int res;

    if(self->killNow)
        return;

    self->killNow = 1;
    /* Lock the backend to ensure we don't flag the mixer to die and signal the
     * mixer to wake up in between it checking the flag and going to sleep and
     * wait for a wakeup (potentially leading to it never waking back up to see
     * the flag). */
    ALCjackPlayback_lock(self);
    ALCjackPlayback_unlock(self);
    alcnd_signal(&self->Cond);
    althrd_join(self->thread, &res);

    jack_deactivate(self->Client);
}


static ClockLatency ALCjackPlayback_getClockLatency(ALCjackPlayback *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    ClockLatency ret;

    ALCjackPlayback_lock(self);
    ret.ClockTime = GetDeviceClockTime(device);
    ret.Latency = ll_ringbuffer_read_space(self->Ring) * DEVICE_CLOCK_RES /
                  device->Frequency;
    ALCjackPlayback_unlock(self);

    return ret;
}


static void ALCjackPlayback_lock(ALCjackPlayback *self)
{
    almtx_lock(&STATIC_CAST(ALCbackend,self)->mMutex);
}

static void ALCjackPlayback_unlock(ALCjackPlayback *self)
{
    almtx_unlock(&STATIC_CAST(ALCbackend,self)->mMutex);
}


static void jack_msg_handler(const char *message)
{
    WARN("%s\n", message);
}

typedef struct ALCjackBackendFactory {
    DERIVE_FROM_TYPE(ALCbackendFactory);
} ALCjackBackendFactory;
#define ALCJACKBACKENDFACTORY_INITIALIZER { { GET_VTABLE2(ALCjackBackendFactory, ALCbackendFactory) } }

static ALCboolean ALCjackBackendFactory_init(ALCjackBackendFactory* UNUSED(self))
{
    jack_client_t *client;
    jack_status_t status;

    if(!jack_load())
        return ALC_FALSE;

    if(!GetConfigValueBool(NULL, "jack", "spawn-server", 0))
        ClientOptions |= JackNoStartServer;

    jack_set_error_function(jack_msg_handler);
    client = jack_client_open("alsoft", ClientOptions, &status, NULL);
    jack_set_error_function(NULL);
    if(client == NULL)
    {
        WARN("jack_client_open() failed, 0x%02x\n", status);
        if((status&JackServerFailed) && !(ClientOptions&JackNoStartServer))
            ERR("Unable to connect to JACK server\n");
        return ALC_FALSE;
    }

    jack_client_close(client);
    return ALC_TRUE;
}

static void ALCjackBackendFactory_deinit(ALCjackBackendFactory* UNUSED(self))
{
#ifdef HAVE_DYNLOAD
    if(jack_handle)
        CloseLib(jack_handle);
    jack_handle = NULL;
#endif
}

static ALCboolean ALCjackBackendFactory_querySupport(ALCjackBackendFactory* UNUSED(self), ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
        return ALC_TRUE;
    return ALC_FALSE;
}

static void ALCjackBackendFactory_probe(ALCjackBackendFactory* UNUSED(self), enum DevProbe type)
{
    switch(type)
    {
        case ALL_DEVICE_PROBE:
            AppendAllDevicesList(jackDevice);
            break;

        case CAPTURE_DEVICE_PROBE:
            break;
    }
}

static ALCbackend* ALCjackBackendFactory_createBackend(ALCjackBackendFactory* UNUSED(self), ALCdevice *device, ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
    {
        ALCjackPlayback *backend;
        NEW_OBJ(backend, ALCjackPlayback)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    return NULL;
}

DEFINE_ALCBACKENDFACTORY_VTABLE(ALCjackBackendFactory);


ALCbackendFactory *ALCjackBackendFactory_getFactory(void)
{
    static ALCjackBackendFactory factory = ALCJACKBACKENDFACTORY_INITIALIZER;
    return STATIC_CAST(ALCbackendFactory, &factory);
}
