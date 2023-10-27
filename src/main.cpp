#include <iostream>
#include "SFML/Audio.hpp"
#include "modules/Rand.h"
#include "modules/ThreadPool.h"
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

int sampleRate = 44100;
int bufferSize = 44100 / 128;

class Oscillator {
private:
    bool isPlaying = false;
    float frequency = 440.0f;
    float phase = 0;
    float amplitude = 1;
public:
    Oscillator() = default;

    [[nodiscard]] std::vector<short> getSamples(int numSamples) {
        float phaseIncrement = 2 * static_cast<float>(M_PI) * frequency / static_cast<float>(sampleRate);
        std::vector<short> samples(numSamples);
        for (int i = 0; i < numSamples; i++) {
            short sampleValue;
            if (isPlaying) {
                sampleValue = static_cast<short>(std::sin(phase) * 32767 * amplitude);
                phase += phaseIncrement;
                if (phase >= M_PI * 2) {
                    phase -= M_PI * 2;
                }
            } else {
                sampleValue = 0;
            }
            samples[i] = sampleValue;
        }

        return samples;
    }

    void stop() {
        isPlaying = false;
    }

    void play() {
        isPlaying = true;
    }

    void setFrequency(float value) {
        if (frequency < 1 || frequency > static_cast<float>(sampleRate) / 2) {
            throw std::runtime_error("Setting osc frequency out of range " + std::to_string(value));
        }
        frequency = value;
    }

    [[nodiscard]] float getFrequency()const {
        return frequency;
    }

    void setAmplitude(float value) {
        if (value < 0 || value > 1) {
            throw std::runtime_error("Setting osc amplitude out of range " + std::to_string(value));
        }
        amplitude = value;
    }
};

class SampleGenerator {
public:
    std::vector<Oscillator> oscillators;

    explicit SampleGenerator(int polyphony) {
        oscillators.resize(polyphony);
    }

    std::vector<short> getSamples(int numSamples) {
        std::vector<short> samples(numSamples);
        for (auto &oscillator: oscillators) {
            std::vector<short> oscSamples = oscillator.getSamples(numSamples);
            for (int i = 0; i < oscSamples.size(); i++) {
                samples[i] = static_cast<short>(samples[i] + oscSamples[i]);
            }
        }
        return samples;
    };
};

// custom audio stream that plays a loaded buffer
class MyStream : public sf::SoundStream {
public:
    MyStream(SampleGenerator &sampleGenerator, ThreadPool& tp) : sampleGenerator(sampleGenerator), sf::SoundStream(), tp(tp) {
        initialize(1, sampleRate);
        bufferA.resize(bufferSize);
        bufferA = sampleGenerator.getSamples(bufferSize);
        bufferB.resize(bufferSize);
    }

private:
    std::vector<short> bufferA;
    std::vector<short> bufferB;
    SampleGenerator &sampleGenerator;
    ThreadPool& tp;

    bool onGetData(Chunk &chunk) override {
        chunk.samples = bufferA.data();
        chunk.sampleCount = bufferSize;

        tp.addTask([this](){
            bufferB = sampleGenerator.getSamples(bufferSize);
            std::swap(bufferA, bufferB);
        });
        return true;

    }

    void onSeek(sf::Time timeOffset) override {}
};

//SampleGenerator getSawGenerator() {
//    RNGf floatGen{};
//    SampleGenerator sampleGenerator{100};
//    float baseFreq1 = 440.0f;
//    float prevFreq1 = baseFreq1;
//    float baseAmplitude1 = 0.2;
//    for (int i = 0; i < sampleGenerator.oscillators.size() / 2; i++) {
//        auto &oscillator = sampleGenerator.oscillators[i];
//        oscillator.setFrequency(prevFreq1);
//        oscillator.play();
//        oscillator.setAmplitude(baseAmplitude1 / static_cast<float>(i + 1));
//        prevFreq1 = prevFreq1 + baseFreq1;
//        if (prevFreq1 > static_cast<float>(sampleRate) / 2) break;
//    }
//}

int main() {

    RNGf floatGen{};
    SampleGenerator sampleGenerator{1};
    sampleGenerator.oscillators[0].play();
    sampleGenerator.oscillators[0].setFrequency(220);
    sampleGenerator.oscillators[0].setAmplitude(0.3);

    ThreadPool tp{1};

    MyStream stream{sampleGenerator, tp};
    stream.play();
//    sf::sleep(sf::milliseconds(10000));

    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Verlet",
                            sf::Style::Default, settings);
    window.setFramerateLimit(60);

    while (window.isOpen()) {

        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Right) {
                    sampleGenerator.oscillators[0].setFrequency(sampleGenerator.oscillators[0].getFrequency() + 2);
                }
            }
        }
    }

    return 0;
}
