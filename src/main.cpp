#include <Arduino.h>

const unsigned long DEBOUNCE_DELAY=50;

enum class Action {
    Nothing,
    Toggle,
};

struct Color {
    int button_pin;
    int led_pin;
    int state { LOW };

    int button_state { 0 };
    int last_button_state { 0 };
    unsigned long last_button_reading { 0 };

    Color(int button, int led)
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
                    ret = Action::Toggle;
                }
            }
        }
        last_button_state = reading;
        return ret;
    }

    void flash()
    {
        state = (state == LOW) ? HIGH : LOW;
        digitalWrite(led_pin, state);
    }
};

struct Colors {
    static Colors* the() {
        if (!m_instance)
            m_instance = new Colors();
        return m_instance;
    }

    Colors()
    {
        colors[0] = new Color(9, 0);
        colors[1] = new Color(10, 1);
        colors[2] = new Color(11, 2);
        colors[3] = new Color(12, 3);
    }

    void all_off()
    {
        for (int ix = 0; ix < 4; ++ix) {
            colors[ix]->led_off();
        }
    }

    void setup()
    {
        for (int ix = 0; ix < 4; ++ix) {
            colors[ix]->setup_pins();
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

        if (current_ts - prev_ts >= 1000) {
            prev_ts = current_ts;
            if (current >= 0) {
                colors[current]->flash();
            }
        }
    }

    Color *colors[4];

private:
    static Colors *m_instance;
    unsigned long prev_ts = 0;
    int current = -1;
};

Colors* Colors::m_instance = nullptr;


// the setup function runs once when you press reset or power the board
void setup()
{
    Colors::the()->setup();
    // Colors::the()->colors[0].led_on();
}

// the loop function runs over and over again forever
void loop()
{
    Colors::the()->loop();
}
