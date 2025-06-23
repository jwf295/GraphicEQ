/*
  ==============================================================================

    EQBand.h
    Created: 22 Oct 2024 2:10:21pm
    Author:  Jeremy Freeman

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class EQBand : public juce::Component,
               public juce::Slider::Listener {
public:
    EQBand();
    ~EQBand();
    void resized() override;
    void initializeVTS(juce::AudioProcessorValueTreeState& vts, const juce::String& paramID);
    void prepare(float frequency, int sampleRate, float gain);
    void process(juce::dsp::AudioBlock<float>& block);
    void reset();
    void setSampleRate(int sampleRate);
    void setInitialGain(float initialGain);
    void setGainAttachment(std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment);
    float getGain();
    juce::Slider& getBandSlider();
private:
    void sliderValueChanged(juce::Slider *slider) override;
    std::vector<juce::IIRFilter> filter;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    int numChannels, sampleRate;
    float frequency, gain;
    juce::Slider bandSlider;
    juce::Label bandLabel;
};

