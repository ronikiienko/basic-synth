#include <iostream>
#include "SFML/Audio.hpp"
#include "modules/Rand.h"

int sampleRate = 44100;
int bufferSize = 44100 * 1;

class Oscillator {
private:
    bool isPlaying = true;
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
    explicit MyStream(SampleGenerator &sampleGenerator) : sampleGenerator(sampleGenerator), sf::SoundStream() {
        initialize(1, sampleRate);
    }

private:
    SampleGenerator &sampleGenerator;

    bool onGetData(Chunk &chunk) override {
        std::vector<short> generatedSamples = sampleGenerator.getSamples(bufferSize);
        chunk.samples = generatedSamples.data();
        chunk.sampleCount = bufferSize;
        return true;
    }

    void onSeek(sf::Time timeOffset) override {}
};

int main() {
    SampleGenerator sampleGenerator{3};
    sampleGenerator.oscillators[0].setFrequency(200);
    sampleGenerator.oscillators[0].setAmplitude(0.2);
    sampleGenerator.oscillators[1].setFrequency(202.0f);
    sampleGenerator.oscillators[1].setAmplitude(0.2);
    sampleGenerator.oscillators[2].setFrequency(50.0f * 2.0);
    sampleGenerator.oscillators[2].setAmplitude(0.2);
    MyStream stream{sampleGenerator};
    stream.play();
    sf::sleep(sf::milliseconds(10000));

    return 0;
}
