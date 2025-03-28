#include "PluginProcessor.h"
#include "PluginEditor.h"

ReverbWavefolderAudioProcessor::ReverbWavefolderAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout())
{
    // Get parameter pointers
    // Reverb parameters
    sizeParam = parameters.getRawParameterValue("size");
    decayParam = parameters.getRawParameterValue("decay");
    diffusionParam = parameters.getRawParameterValue("diffusion");
    densityParam = parameters.getRawParameterValue("density");
    lowEQParam = parameters.getRawParameterValue("lowEQ");
    midEQParam = parameters.getRawParameterValue("midEQ");
    highEQParam = parameters.getRawParameterValue("highEQ");
    
    // Wavefolder parameters
    driveParam = parameters.getRawParameterValue("drive");
    thresholdParam = parameters.getRawParameterValue("threshold");
    offsetParam = parameters.getRawParameterValue("offset");
    fundamentalParam = parameters.getRawParameterValue("fundamental");
    foldSymmetryParam = parameters.getRawParameterValue("foldSymmetry");
    waveformShapeParam = parameters.getRawParameterValue("waveformShape");
    
    // Additional parameters
    dryWetParam = parameters.getRawParameterValue("dryWet");
    preDelayParam = parameters.getRawParameterValue("preDelay");
    wavefoldPositionParam = parameters.getRawParameterValue("wavefoldPosition");
}

ReverbWavefolderAudioProcessor::~ReverbWavefolderAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout ReverbWavefolderAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Reverb parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>("size", "Size", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", 0.1f, 20.0f, 2.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("diffusion", "Diffusion", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("density", "Density", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("lowEQ", "Low EQ", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("midEQ", "Mid EQ", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("highEQ", "High EQ", 0.0f, 1.0f, 0.5f));
    
    // Wavefolder parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", 1.0f, 10.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", 0.1f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("offset", "Offset", -1.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("fundamental", "Fundamental", 20.0f, 5000.0f, 1000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("foldSymmetry", "Fold Symmetry", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("waveformShape", "Waveform Shape", 0.0f, 1.0f, 0.5f));
    
    // Additional parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>("dryWet", "Dry/Wet", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("preDelay", "Pre-Delay", 0.0f, 500.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("wavefoldPosition", "Wavefold Position",
        juce::StringArray("Pre-Reverb", "In-Reverb Loop", "Post-Reverb"), 2));
    
    return layout;
}

void ReverbWavefolderAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Set up pre-delay
    preDelay.reset();
    preDelay.prepare({ sampleRate, (juce::uint32) samplesPerBlock, 2 });
    preDelay.setMaximumDelayInSamples(sampleRate * 0.5); // Max 500ms pre-delay
    
    // Set up reverb
    updateReverbParameters();
   
    juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = samplesPerBlock;
        spec.numChannels = getTotalNumInputChannels();
        
        reverb.prepare(spec);
        reverb.reset();
    
    // Set up wavefolder
    wavefolder.prepare(spec);
    
    // Prepare buffers
    dryBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    wetBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    preFoldBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    postFoldBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
}

void ReverbWavefolderAudioProcessor::releaseResources() {}

void ReverbWavefolderAudioProcessor::updateReverbParameters()
{
    reverbParams.roomSize = *sizeParam;
    reverbParams.damping = 1.0f - *decayParam / 20.0f; // Convert decay time to damping
    
    // Ensure damping isn't too low at high decay values
    // Apply a minimum damping value
    if (reverbParams.damping < 0.05f)
            reverbParams.damping = 0.05f;
    
    reverbParams.width = *diffusionParam;
    reverbParams.wetLevel = 1.0f; // Handle dry/wet separately
    reverbParams.dryLevel = 0.0f; // Handle dry/wet separately
    reverb.setParameters(reverbParams);
}



void ReverbWavefolderAudioProcessor::applyWavefolding(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    const float drive = *driveParam;
    const float threshold = *thresholdParam;
    const float offset = *offsetParam;
    const float fold = *foldSymmetryParam;
    const float shape = *waveformShapeParam;
    const float fundamental = *fundamentalParam;
    
    // Use custom wavefolder class
    wavefolder.processBlock(buffer, drive, threshold, offset, fold, shape, fundamental);
    
    // Add DC blocking (important for pre-reverb position)
    static float prevIn[2] = {0.0f, 0.0f};
    static float prevOut[2] = {0.0f, 0.0f};
    
    const float dcBlockCoeff = 0.995f;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float input = channelData[sample];
            channelData[sample] = input - prevIn[channel] + dcBlockCoeff * prevOut[channel];
            prevIn[channel] = input;
            prevOut[channel] = channelData[sample];
        }
    }
}
void ReverbWavefolderAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // Update the reverb parameters
    updateReverbParameters();
    
    // Save dry buffer for later mixing
    dryBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);
    if (numChannels > 1)
        dryBuffer.copyFrom(1, 0, buffer, 1, 0, numSamples);
        
    // Apply pre-delay
    const float preDelayMs = *preDelayParam;
    const float preDelayInSamples = preDelayMs * 0.001f * getSampleRate();
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            preDelay.pushSample(channel, channelData[sample]);
            channelData[sample] = preDelay.popSample(channel, preDelayInSamples);
        }
    }
    
    // Copy the buffer for potential pre-reverb wavefolding
    wetBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);
    if (numChannels > 1)
        wetBuffer.copyFrom(1, 0, buffer, 1, 0, numSamples);
    
    // Get the wavefold position
    const int wavefoldPos = static_cast<int>(*wavefoldPositionParam);
    
    // Apply wavefolding based on position
    if (wavefoldPos == WavefoldPosition::PRE_REVERB)
    {
        applyWavefolding(wetBuffer, 0, numSamples);
    }
    
    // Apply reverb
    juce::dsp::AudioBlock<float> block(wetBuffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
    
    // Apply wavefolding in reverb loop or post-reverb
    if (wavefoldPos == WavefoldPosition::IN_REVERB_LOOP)
    {
        // In-loop folding (simplified implementation - in a real implementation
        // you'd need to integrate the folding into the reverb's feedback loop)
        applyWavefolding(wetBuffer, 0, numSamples);
    }
    else if (wavefoldPos == WavefoldPosition::POST_REVERB)
    {
        applyWavefolding(wetBuffer, 0, numSamples);
    }
    
    // Mix dry and wet signals
    const float wet = *dryWetParam;
    const float dry = 1.0f - wet;
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        const float* dryData = dryBuffer.getReadPointer(channel);
        const float* wetData = wetBuffer.getReadPointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] = dryData[sample] * dry + wetData[sample] * wet;
        }
    }
    
    // Apply noise gate to eliminate ghost signals
    bool hasSignal = false;
    for (int channel = 0; channel < numChannels && !hasSignal; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            
            for (int sample = 0; sample < numSamples; ++sample)
            {
                if (std::abs(channelData[sample]) > noiseGateThreshold)
                {
                    hasSignal = true;
                    silenceCounter = 0;
                    break;
                }
            }
        }
        
        if (!hasSignal)
        {
            silenceCounter += numSamples;
            
            if (silenceCounter > silenceCounterThreshold)
            {
                // Completely silence the output after the threshold is reached
                for (int channel = 0; channel < numChannels; ++channel)
                {
                    buffer.clear(channel, 0, numSamples);
                }
                
                // Also clear the internal state of the reverb to prevent ghost outputs
                if (silenceCounter == silenceCounterThreshold + numSamples)
                {
                    reverb.reset();
                    wavefolder.reset();
                }
            }
        }
}

bool ReverbWavefolderAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Support mono and stereo layouts
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    
    return true;
}

juce::AudioProcessorEditor* ReverbWavefolderAudioProcessor::createEditor()
{
    return new ReverbWavefolderEditor(*this);
}

void ReverbWavefolderAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ReverbWavefolderAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// This creates the plugin instance
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReverbWavefolderAudioProcessor();
}
