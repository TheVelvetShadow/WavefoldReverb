#pragma once

#include <JuceHeader.h>
#include <cmath>

class Wavefolder
{
public:
    Wavefolder() = default;
    ~Wavefolder() = default;
    
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        reset();
    }
    
    void reset()
    {
        phase = 0.0f;
    }
    
    // Main wavefolder processing function
    float process(float input, float drive, float threshold, float offset, float symmetry, float shape, float fundamental)
    {
        // Update phase for fundamental oscillation
        float freq = fundamental;
        phase += freq / sampleRate;
        if (phase >= 1.0f)
            phase -= 1.0f;
        
        // Apply drive to increase gain
        float amplified = input * drive;
        
        // Apply offset before folding
        amplified += offset;
        
        // Apply wavefolding
        float folded = foldSignal(amplified, threshold, symmetry, shape);
        
        // Remove offset
        folded -= offset;
        
        // Normalize output level
        folded /= drive;
        
        return folded;
    }
    
    // Process a buffer of samples
    void processBlock(juce::AudioBuffer<float>& buffer, float drive, float threshold,
                     float offset, float symmetry, float shape, float fundamental)
    {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            
            for (int sample = 0; sample < numSamples; ++sample)
            {
                channelData[sample] = process(channelData[sample], drive, threshold,
                                             offset, symmetry, shape, fundamental);
            }
        }
    }
    
private:
    float sampleRate = 44100.0f;
    float phase = 0.0f;
    
    // Different folding algorithms based on shape parameter
    float foldSignal(float input, float threshold, float symmetry, float shape)
    {
        // Ensure shape is between 0 and 1
        shape = juce::jlimit(0.0f, 1.0f, shape);
        
        // Use shape to blend between different folding algorithms
        if (shape < 0.33f)
        {
            // Simple folder (triangle folding)
            return basicFold(input, threshold, symmetry * 2.0f - 1.0f);
        }
        else if (shape < 0.66f)
        {
            // Sine folder
            return sineFold(input, threshold, symmetry * 2.0f - 1.0f);
        }
        else
        {
            // Hyperbolic tangent folder
            return tanhFold(input, threshold, symmetry * 2.0f - 1.0f);
        }
    }
    
    // Basic triangle folding
    float basicFold(float input, float threshold, float symmetry)
    {
        float output = input;
        
        // Perform folding for values exceeding threshold
        while (std::abs(output) > threshold)
        {
            if (output > threshold)
                output = threshold - (output - threshold);
            else if (output < -threshold)
                output = -threshold + (-output - threshold);
        }
        
        // Apply symmetry
        if (symmetry > 0.0f)
            output *= (1.0f + symmetry * (1.0f - std::abs(output / threshold)));
        else if (symmetry < 0.0f)
            output *= (1.0f + symmetry * std::abs(output / threshold));
            
        return output;
    }
    
    // Sine-based folding
    float sineFold(float input, float threshold, float symmetry)
    {
        // Scale input to work with sin function
        float normInput = input / threshold;
        
        // Apply sine shaping when input exceeds threshold
        if (std::abs(normInput) > 1.0f)
        {
            float sign = (normInput > 0.0f) ? 1.0f : -1.0f;
            float foldedAmount = std::fmod(std::abs(normInput) - 1.0f, 2.0f);
            
            if (foldedAmount > 1.0f)
                foldedAmount = 2.0f - foldedAmount;
                
            // Apply symmetry to the folded amount
            if (symmetry != 0.0f)
                foldedAmount = foldedAmount * (1.0f + symmetry * (foldedAmount < 0.5f ? foldedAmount * 2.0f : (1.0f - foldedAmount) * 2.0f));
                
            // Apply sine shaping
            foldedAmount = std::sin(foldedAmount * juce::MathConstants<float>::pi * 0.5f);
            
            return sign * foldedAmount * threshold;
        }
        
        return input;
    }
    
    // Hyperbolic tangent folding
    float tanhFold(float input, float threshold, float symmetry)
    {
        // Scale input to the threshold
        float normInput = input / threshold;
        
        // Apply tanh shaping for extreme values
        if (std::abs(normInput) > 1.0f)
        {
            float sign = (normInput > 0.0f) ? 1.0f : -1.0f;
            float foldAmount = std::abs(normInput) - 1.0f;
            
            // Create a series of tanh folds
            float foldedAmount = std::tanh(foldAmount);
            
            // Apply symmetry
            if (symmetry > 0.0f)
                foldedAmount *= (1.0f + symmetry * (1.0f - foldedAmount));
            else if (symmetry < 0.0f)
                foldedAmount *= (1.0f + symmetry * foldedAmount);
                
            return sign * (1.0f - foldedAmount) * threshold;
        }
        
        return input;
    }
};
