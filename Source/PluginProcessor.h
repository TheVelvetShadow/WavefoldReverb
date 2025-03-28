#pragma once

#include <JuceHeader.h>
#include "Wavefolder.h" // Include our custom wavefolder

class ReverbWavefolderAudioProcessor : public juce::AudioProcessor
{
public:
    ReverbWavefolderAudioProcessor();
    ~ReverbWavefolderAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return {}; }
    void changeProgramName(int index, const juce::String& newName) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    enum WavefoldPosition {
        PRE_REVERB,
        IN_REVERB_LOOP,
        POST_REVERB
    };

    // Audio Parameters
    juce::AudioProcessorValueTreeState parameters;

private:
    // Reverb parameters
    std::atomic<float>* sizeParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* diffusionParam = nullptr;
    std::atomic<float>* densityParam = nullptr;
    std::atomic<float>* lowEQParam = nullptr;
    std::atomic<float>* midEQParam = nullptr;
    std::atomic<float>* highEQParam = nullptr;
    
    // Wavefolder parameters
    std::atomic<float>* driveParam = nullptr;
    std::atomic<float>* thresholdParam = nullptr;
    std::atomic<float>* offsetParam = nullptr;
    std::atomic<float>* fundamentalParam = nullptr;
    std::atomic<float>* foldSymmetryParam = nullptr;
    std::atomic<float>* waveformShapeParam = nullptr;
    
    // Additional parameters
    std::atomic<float>* dryWetParam = nullptr;
    std::atomic<float>* preDelayParam = nullptr;
    std::atomic<float>* wavefoldPositionParam = nullptr;

    // DSP Components
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> preDelay;
    juce::dsp::Reverb::Parameters reverbParams;
    juce::dsp::Reverb reverb;
    Wavefolder wavefolder;
    
    // Internal buffers
    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> wetBuffer;
    juce::AudioBuffer<float> preFoldBuffer;
    juce::AudioBuffer<float> postFoldBuffer;
    double currentSampleRate = 44100.0;
    
    // Internal methods
    void applyWavefolding(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    void updateReverbParameters();
    
    // Parameter initialization
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbWavefolderAudioProcessor)
};

