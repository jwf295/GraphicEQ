/*
  ==============================================================================

    EQBand.cpp
    Created: 22 Oct 2024 2:10:11pm
    Author:  Jeremy Freeman

  ==============================================================================
*/

#include "EQBand.h"

EQBand::EQBand() {
    bandSlider.setSliderStyle(juce::Slider::LinearVertical);
    bandSlider.setRange(0.01, 3.0);
    bandSlider.setValue(1.0);
    bandSlider.addListener(this);
    bandSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 20);
    gain = 1;
    bandLabel.setJustificationType(juce::Justification::centred);
    bandLabel.setAlwaysOnTop(true);
    addAndMakeVisible(bandLabel);
    addAndMakeVisible(bandSlider);
}

EQBand::~EQBand() {
    gainAttachment.reset();
}

void EQBand::resized() {
    bandLabel.setBounds(0, 0, getWidth(), 20);
    bandSlider.setBounds(0, 20, getWidth(), getHeight() - 40);
}

void EQBand::initializeVTS(juce::AudioProcessorValueTreeState& vts, const juce::String& paramID) {
    if (!gainAttachment) {
        gainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, paramID, bandSlider));
    }
}

void EQBand::prepare(float frequency, int sampleRate, float gain) {
    this->frequency = frequency;
    this->sampleRate = sampleRate;
    this->gain = gain;
    bandLabel.setText(std::to_string((int)frequency), juce::dontSendNotification);
    auto coefficients = juce::IIRCoefficients::makePeakFilter(this->sampleRate, this->frequency, 1.0f, this->gain);
    for (int channel = 0; channel < filter.size(); ++channel) {
        filter[channel].setCoefficients(coefficients);
        reset();
    }
}

void EQBand::process(juce::dsp::AudioBlock<float>& block) {
    if (block.getNumChannels() != filter.size()) {
        filter.resize(block.getNumChannels());
        prepare(frequency, sampleRate, gain);
    }
    for (int channel = 0; channel < block.getNumChannels(); ++channel) {
        auto* samples = block.getChannelPointer(channel);
        filter[channel].processSamples(samples, static_cast<int>(block.getNumSamples()));
    }
}

void EQBand::reset() {
    for (int channel = 0; channel < filter.size(); ++channel) {
        filter[channel].reset();
    }
}

void EQBand::setSampleRate(int sampleRate) {
    this->sampleRate = sampleRate;
}

void EQBand::setInitialGain(float initialGain) {
    gain = initialGain;
    bandSlider.setValue(gain, juce::dontSendNotification);
}

void EQBand::setGainAttachment(std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment) {
    this->gainAttachment = std::move(gainAttachment);
}

float EQBand::getGain() {
    return gain;
}

juce::Slider& EQBand::getBandSlider() {
    return bandSlider;
}

void EQBand::sliderValueChanged(juce::Slider *slider) {
    if (slider == &bandSlider) {
        gain = bandSlider.getValue();
        prepare(frequency, sampleRate, gain);
    }
}
