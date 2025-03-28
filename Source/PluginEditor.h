#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class ReverbWavefolderEditor : public juce::AudioProcessorEditor
{
public:
    ReverbWavefolderEditor(ReverbWavefolderAudioProcessor& p)
        : AudioProcessorEditor(&p), processor(p)
    {
        // Set up parameter sliders and labels
        
        // Reverb section
        addSliderAndLabel("Size", sizeSlider, sizeLabel);
        addSliderAndLabel("Decay", decaySlider, decayLabel);
        addSliderAndLabel("Diffusion", diffusionSlider, diffusionLabel);
        addSliderAndLabel("Density", densitySlider, densityLabel);
        addSliderAndLabel("Low EQ", lowEQSlider, lowEQLabel);
        addSliderAndLabel("Mid EQ", midEQSlider, midEQLabel);
        addSliderAndLabel("High EQ", highEQSlider, highEQLabel);
        
        // Wavefolder section
        addSliderAndLabel("Drive", driveSlider, driveLabel);
        addSliderAndLabel("Threshold", thresholdSlider, thresholdLabel);
        addSliderAndLabel("Offset", offsetSlider, offsetLabel);
        addSliderAndLabel("Fundamental", fundamentalSlider, fundamentalLabel);
        addSliderAndLabel("Fold Symmetry", foldSymmetrySlider, foldSymmetryLabel);
        addSliderAndLabel("Waveform Shape", waveformShapeSlider, waveformShapeLabel);
        
        // Additional controls
        addSliderAndLabel("Dry/Wet", dryWetSlider, dryWetLabel);
        addSliderAndLabel("Pre-Delay", preDelaySlider, preDelayLabel);
        
        // Wavefold position combo box
        wavefoldPosLabel.setText("Wavefold Position", juce::dontSendNotification);
        addAndMakeVisible(wavefoldPosLabel);
        
        wavefoldPosCombo.addItem("Pre-Reverb", 1);
        wavefoldPosCombo.addItem("In-Reverb Loop", 2);
        wavefoldPosCombo.addItem("Post-Reverb", 3);
        wavefoldPosCombo.setSelectedId(3); // Default to Post-Reverb
        addAndMakeVisible(wavefoldPosCombo);
        
        // Set up parameter attachments
        sizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "size", sizeSlider);
        decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "decay", decaySlider);
        diffusionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "diffusion", diffusionSlider);
        densityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "density", densitySlider);
        lowEQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "lowEQ", lowEQSlider);
        midEQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "midEQ", midEQSlider);
        highEQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "highEQ", highEQSlider);
        
        driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "drive", driveSlider);
        thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "threshold", thresholdSlider);
        offsetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "offset", offsetSlider);
        fundamentalAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "fundamental", fundamentalSlider);
        foldSymmetryAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "foldSymmetry", foldSymmetrySlider);
        waveformShapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "waveformShape", waveformShapeSlider);
        
        dryWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "dryWet", dryWetSlider);
        preDelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "preDelay", preDelaySlider);
        wavefoldPosAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processor.parameters, "wavefoldPosition", wavefoldPosCombo);
            
        // Set window size
        setSize(800, 600);
    }

    ~ReverbWavefolderEditor() override = default;

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey);
        
        // Draw section headers
        g.setColour(juce::Colours::white);
        g.setFont(18.0f);
        
        g.drawText("Reverb", 20, 10, 350, 30, juce::Justification::left);
        g.drawText("Wavefolder", 420, 10, 350, 30, juce::Justification::left);
        g.drawText("Mix", 20, 320, 350, 30, juce::Justification::left);
        
        // Draw section dividers
        g.setColour(juce::Colours::lightgrey);
        g.drawLine(20, 40, 780, 40, 1.0f);
        g.drawLine(20, 350, 780, 350, 1.0f);
        g.drawLine(400, 40, 400, 320, 1.0f);
    }

    void resized() override
    {
        const int labelWidth = 120;
        const int sliderWidth = 250;
        const int controlHeight = 30;
        const int margin = 10;
        
        // Layout for reverb section
        int y = 50;
        sizeLabel.setBounds(20, y, labelWidth, controlHeight);
        sizeSlider.setBounds(20 + labelWidth, y, sliderWidth, controlHeight);
        
        y += controlHeight + margin;
        decayLabel.setBounds(20, y, labelWidth, controlHeight);
        decaySlider.setBounds(20 + labelWidth, y, sliderWidth, controlHeight);
        
        y += controlHeight + margin;
        diffusionLabel.setBounds(20, y, labelWidth, controlHeight);
        diffusionSlider.setBounds(20 + labelWidth, y, sliderWidth, controlHeight);
        
        y += controlHeight + margin;
        densityLabel.setBounds(20, y, labelWidth, controlHeight);
        densitySlider.setBounds(20 + labelWidth, y, sliderWidth, controlHeight);
        
        y += controlHeight + margin;
        lowEQLabel.setBounds(20, y, labelWidth, controlHeight);
        lowEQSlider.setBounds(20 + labelWidth, y, sliderWidth, controlHeight);
        
        y += controlHeight + margin;
        midEQLabel.setBounds(20, y, labelWidth, controlHeight);
        midEQSlider.setBounds(20 + labelWidth, y, sliderWidth, controlHeight);
        
        y += controlHeight + margin;
        highEQLabel.setBounds(20, y, labelWidth, controlHeight);
        highEQSlider.setBounds(20 + labelWidth, y, sliderWidth, controlHeight);
        
        // Layout for wavefolder section
        y = 50;
        driveLabel.setBounds(420, y, labelWidth, controlHeight);
        driveSlider.setBounds(420 + labelWidth, y, sliderWidth, controlHeight);
        
        y += controlHeight + margin;
        thresholdLabel.setBounds(420, y, labelWidth, controlHeight);
        thresholdSlider.setBounds(420 + labelWidth, y, sliderWidth, controlHeight);
        
        y += controlHeight + margin;
        offsetLabel.setBounds(420, y, labelWidth, controlHeight);
        offsetSlider.setBounds(420 + labelWidth, y, sliderWidth, controlHeight);
        
        y += controlHeight + margin;
        fundamentalLabel.setBounds(420, y, labelWidth, controlHeight);
        fundamentalSlider.setBounds(420 + labelWidth, y, sliderWidth, controlHeight);
        
        y += controlHeight + margin;
        foldSymmetryLabel.setBounds(420, y, labelWidth, controlHeight);
        foldSymmetrySlider.setBounds(420 + labelWidth, y, sliderWidth, controlHeight);
        
        y += controlHeight + margin;
        waveformShapeLabel.setBounds(420, y, labelWidth, controlHeight);
        waveformShapeSlider.setBounds(420 + labelWidth, y, sliderWidth, controlHeight);
        
        // Layout for mix section
        y = 370;
        dryWetLabel.setBounds(20, y, labelWidth, controlHeight);
        dryWetSlider.setBounds(20 + labelWidth, y, sliderWidth, controlHeight);
        
        y += controlHeight + margin;
        preDelayLabel.setBounds(20, y, labelWidth, controlHeight);
        preDelaySlider.setBounds(20 + labelWidth, y, sliderWidth, controlHeight);
        
        y += controlHeight + margin;
        wavefoldPosLabel.setBounds(20, y, labelWidth, controlHeight);
        wavefoldPosCombo.setBounds(20 + labelWidth, y, sliderWidth, controlHeight);
    }

private:
    void addSliderAndLabel(const juce::String& labelText, juce::Slider& slider, juce::Label& label)
    {
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::right);
        addAndMakeVisible(label);
        
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
        addAndMakeVisible(slider);
    }

    // Reference to the processor
    ReverbWavefolderAudioProcessor& processor;
    
    // Reverb controls
    juce::Slider sizeSlider, decaySlider, diffusionSlider, densitySlider;
    juce::Slider lowEQSlider, midEQSlider, highEQSlider;
    juce::Label sizeLabel, decayLabel, diffusionLabel, densityLabel;
    juce::Label lowEQLabel, midEQLabel, highEQLabel;
    
    // Wavefolder controls
    juce::Slider driveSlider, thresholdSlider, offsetSlider, fundamentalSlider;
    juce::Slider foldSymmetrySlider, waveformShapeSlider;
    juce::Label driveLabel, thresholdLabel, offsetLabel, fundamentalLabel;
    juce::Label foldSymmetryLabel, waveformShapeLabel;
    
    // Additional controls
    juce::Slider dryWetSlider, preDelaySlider;
    juce::Label dryWetLabel, preDelayLabel;
    juce::ComboBox wavefoldPosCombo;
    juce::Label wavefoldPosLabel;
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> diffusionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> densityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowEQAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midEQAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highEQAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> offsetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fundamentalAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> foldSymmetryAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> waveformShapeAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preDelayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> wavefoldPosAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbWavefolderEditor)
};
