#include <Arduino.h>

const unsigned long DEBOUNCE_DELAY=50;

enum class Action {
    Nothing,
    Toggle,
};

enum class Mode {
    Off,
    Solid,
    Flashing,
};

struct LED {
    int button_pin;
    int led_pin;
    int state { LOW };
    Mode mode { Mode::Off };

    int button_state { 0 };
    int last_button_state { 0 };
    unsigned long last_button_reading { 0 };

    LED(int button, int led)
        : button_pin(button)
        , led_pin(led)
    {
        led_off();
    }

    void led_off()
    {
        state = LOW;
        digitalWrite(led_pin, LOW);
    }

    void led_on()
    {
        digitalWrite(led_pin, HIGH);
        state = HIGH;
    }

    void setup_pins()
    {
        pinMode(button_pin, INPUT);
        pinMode(led_pin, OUTPUT);
        led_off();
    }


    void flash()
    {
        state = (state == LOW) ? HIGH : LOW;
        digitalWrite(led_pin, state);
    }

    Action check_button()
    {
        Action ret = Action::Nothing;
        int reading = digitalRead(button_pin);
        if (reading != last_button_state) {
            last_button_reading = millis();
        }

        if ((millis() - last_button_reading) > DEBOUNCE_DELAY) {
            if (reading != button_state) {
                button_state = reading;
                if (button_state == HIGH) {
                    ret = on_button_pressed();
                }
            }
        }
        last_button_state = reading;
        return ret;
    }

    virtual Action on_button_pressed() = 0;
};

struct Color : public LED {
    Color(int button, int led)
        : LED(button, led)
    {
        mode = Mode::Flashing;
    }

    Action on_button_pressed() override
    {
        return Action::Toggle;
    }
};

struct White : public LED {
    Mode mode { Mode::Off };

    White(int button, int led) : LED(button, led)
    {
    }


    Action on_button_pressed() override
    {
        Action ret = Action::Nothing;
        switch (mode) {
        case Mode::Off:
            mode = Mode::Flashing;
            ret = Action::Toggle;
            break;
        case Mode::Flashing:
            mode = Mode::Solid;
            ret = Action::Nothing;
            break;
        case Mode::Solid:
            mode = Mode::Off;
            ret = Action::Toggle;
            break;
        }
        return ret;
    }
};

struct SmartJock {
    static SmartJock* the() {
        if (!m_instance)
            m_instance = new SmartJock();
        return m_instance;
    }

    SmartJock() : white(new White(13, 4))
    {
        colors[0] = new Color(9, 0);
        colors[1] = new Color(10, 1);
        colors[2] = new Color(11, 2);
        colors[3] = new Color(12, 3);

        for (int ix = 0; ix < 4; ++ix) {
            leds[ix] = colors[ix];
        }
        leds[4] = white;
    }

    void setup()
    {
        for (auto *led : leds) {
            led->setup_pins();
        }
    }

    void loop()
    {
        unsigned long current_ts = millis();

        for (int ix = 0; ix < 4; ++ix) {
            if (colors[ix]->check_button() == Action::Toggle) {
                if (current != ix) {
                    if (current >= 0)
                        colors[current]->led_off();
                    current = ix;
                    colors[ix]->led_on();
                } else {
                    current = -1;
                    colors[ix]->led_off();
                }
                break;
            }
        }
        if (white->check_button() == Action::Toggle) {
            switch (white->mode) {
            case Mode::Off:
                white->led_off();
                break;
            case Mode::Solid:
                white->led_on();
                break;
            case Mode::Flashing:
                break;
            }
        }

        if (current_ts - prev_ts >= 1000) {
            prev_ts = current_ts;
            if (current >= 0) {
                colors[current]->flash();
            }
            if (white->mode == Mode::Flashing) {
                white->flash();
            }
        }
    }

    Color *colors[4];
    White *white;
    LED   *leds[5];

private:
    static SmartJock*m_instance;
    unsigned long prev_ts = 0;
    int current = -1;
};

SmartJock* SmartJock::m_instance = nullptr;

void setup()
{
    SmartJock::the()->setup();
}

void loop()
{
    SmartJock::the()->loop();
}
