#ifndef DEVICE_KEYMAP_H
#define DEVICE_KEYMAP_H

#include <stdint.h>
#include <cstddef>

#define KEYMAP_ALL_KEYS_UP_Val      1
#define KEYMAP_ALL_KEYS_UP_POS      28
#define KEYMAP_ALL_KEYS_UP_MASK(x)  ((uint32_t)x << KEYMAP_ALL_KEYS_UP_POS)
#define KEYMAP_ALL_KEYS_UP          KEYMAP_ALL_KEYS_UP_MASK(KEYMAP_ALL_KEYS_UP_Val)

#define KEYMAP_NORMAL_KEY_Val       0
#define KEYMAP_MODIFIER_KEY_Val     1
#define KEYMAP_MODIFIER_POS         29
#define KEYMAP_MODIFIER_MASK(x)     ((uint32_t)x << KEYMAP_MODIFIER_POS)
#define KEYMAP_MODIFIER_KEY         KEYMAP_MODIFIER_MASK(KEYMAP_MODIFIER_KEY_Val)

#define KEYMAP_MEDIA_KEY_Val        1
#define KEYMAP_MEDIA_POS            30
#define KEYMAP_MEDIA_MASK(x)        ((uint32_t)x << KEYMAP_MEDIA_POS)
#define KEYMAP_MEDIA_KEY            KEYMAP_MEDIA_MASK(KEYMAP_MEDIA_KEY_Val)

#define KEYMAP_KEY_UP_Val           0
#define KEYMAP_KEY_DOWN_Val         1
#define KEYMAP_KEY_DOWN_POS         31
#define KEYMAP_KEY_DOWN_MASK(x)     ((uint32_t)x << KEYMAP_KEY_DOWN_POS)
#define KEYMAP_KEY_DOWN             KEYMAP_KEY_DOWN_MASK(KEYMAP_KEY_DOWN_Val)
#define KEYMAP_KEY_UP               KEYMAP_KEY_DOWN_MASK(KEYMAP_KEY_UP_Val)

#define KEYMAP_REGISTER(x) { .seq = x, .length = sizeof(x)/sizeof(Key) }

namespace codal
{
    typedef enum
    {
        Mute,
        VolumeUp,
        VolumeDown,
        PlayPause,
        Stop,
        PreviousTrack,
        NextTrack,
        Mail,
        Calculator,
        WebSearch,
        WebHome,
        WebFavourites,
        WebRefresh,
        WebStop,
        WebForward,
        WebBack
    } MediaKey;

    typedef enum
    {
        F1Key = 0x3a,   /* F1 key */
        F2Key,         /* F2 key */
        F3Key,         /* F3 key */
        F4Key,         /* F4 key */
        F5Key,         /* F5 key */
        F6Key,         /* F6 key */
        F7Key,         /* F7 key */
        F8Key,         /* F8 key */
        F9Key,         /* F9 key */
        F10Key,        /* F10 key */
        F11Key,        /* F11 key */
        F12Key,        /* F12 key */

        PrintScreen = 0x46,   /* Print Screen key */
        ScrollLock,    /* Scroll lock */
        Pause,      /* pause key */
        Insert,       /* Insert key */
        Home,         /* Home key */
        PageUp,        /* Page Up key */
        DeleteForward, /* Delete Forward */
        End,            /* End */
        PageDown,      /* Page Down key */

        RightArrow = 0x4F,        /* Right arrow */
        LeftArrow,         /* Left arrow */
        DownArrow,         /* Down arrow */
        UpArrow,           /* Up arrow */
    } FunctionKey;

    typedef union
    {
        struct {
            uint16_t code:16;
            uint16_t reserved:12;
            bool allKeysUp:1;
            bool isModifier:1;
            bool isMedia:1;
            bool isKeyDown:1;
        } bit;
        uint32_t reg;
    } Key;

    typedef struct{
        const Key* seq;
        uint8_t length;
    } KeySequence;

    class KeyMap
    {
    protected:
        const KeySequence* map;

    public:
        uint32_t length;

        KeyMap()
        {
            length = 0;
        }

        KeyMap(const KeySequence* seq, uint32_t len)
        {
            map = seq;
            length = len;
        };

        Key getMediaKey(MediaKey t)
        {
            Key k;

            // shift t by 8 (network byte order)
            k.reg = KEYMAP_KEY_DOWN | KEYMAP_MEDIA_KEY | (1 << t);

            return k;
        }

        Key getFunctionKey(FunctionKey t)
        {
            Key k;

            k.reg = KEYMAP_KEY_DOWN | t;

            return k;
        }

        virtual const KeySequence* mapCharacter(uint16_t c)
        {
            return NULL;
        };
    };
}

#endif