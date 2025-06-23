/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GraphicEQAudioProcessorEditor::GraphicEQAudioProcessorEditor (GraphicEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p) {
        for (int bandNum = 0; bandNum < 10; ++bandNum) {
            addAndMakeVisible(audioProcessor.getEQBand(bandNum));
        }
        // Make sure that before the constructor has finished, you've set the
        // editor's size to whatever you need it to be.
        freqLabel.setText("Band Center Frequencies", juce::dontSendNotification);
        gainLabel.setText("Band Gain, dB", juce::dontSendNotification);
        freqLabel.setJustificationType(juce::Justification::centred);
        gainLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(freqLabel);
        addAndMakeVisible(gainLabel);
        setSize(520, 260);
}

GraphicEQAudioProcessorEditor::~GraphicEQAudioProcessorEditor() {
}

//==============================================================================
void GraphicEQAudioProcessorEditor::paint (juce::Graphics& g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
}

void GraphicEQAudioProcessorEditor::resized() {
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds();
    freqLabel.setBounds(area.removeFromTop(30));
    gainLabel.setBounds(area.removeFromBottom(30));
    int spacing = 50;
    int numBands = 10;
    int bandWidth = 50;
    int bandHeight = 200;
    for (int bandNum = 0; bandNum < numBands; bandNum++) {
        audioProcessor.getEQBand(bandNum).setBounds(
            10 + bandNum * spacing,
            30,
            bandWidth,
            bandHeight
        );
    }
}
