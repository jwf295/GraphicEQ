/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GraphicEQAudioProcessor::GraphicEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    parameters(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
}

GraphicEQAudioProcessor::~GraphicEQAudioProcessor() {
}

//==============================================================================
const juce::String GraphicEQAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool GraphicEQAudioProcessor::acceptsMidi() const {
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GraphicEQAudioProcessor::producesMidi() const {
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GraphicEQAudioProcessor::isMidiEffect() const {
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GraphicEQAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int GraphicEQAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GraphicEQAudioProcessor::getCurrentProgram() {
    return 0;
}

void GraphicEQAudioProcessor::setCurrentProgram (int index) {
}

const juce::String GraphicEQAudioProcessor::getProgramName (int index) {
    return {};
}

void GraphicEQAudioProcessor::changeProgramName (int index, const juce::String& newName) {
}

//==============================================================================
void GraphicEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    const std::array<int, 10> freqArray = {32, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};
    for (int bandNum = 0; bandNum < 10; ++bandNum) {
        eqBands[bandNum].prepare(freqArray[bandNum], sampleRate, eqBands[bandNum].getGain());
        auto paramID = "GainBand" + juce::String(bandNum + 1);
        eqBands[bandNum].initializeVTS(parameters, paramID);
    }
}

void GraphicEQAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GraphicEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const {
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void GraphicEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    juce::dsp::AudioBlock<float> audioBlock(buffer);
    for (int bandNum = 0; bandNum < 10; ++bandNum) {
        eqBands[bandNum].process(audioBlock);
    }
}

//==============================================================================
bool GraphicEQAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GraphicEQAudioProcessor::createEditor() {
    return new GraphicEQAudioProcessorEditor (*this);
}

//==============================================================================
void GraphicEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void GraphicEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(parameters.state.getType())) {
            parameters.replaceState(juce::ValueTree::fromXml (*xmlState));
        }
    }
}

EQBand& GraphicEQAudioProcessor::getEQBand(int index) {
    if (index < 0 || index >= 10) {
        throw std::out_of_range("Index out of bounds for EQ bands.");
    }
    return eqBands[index];
}

juce::AudioProcessorValueTreeState& GraphicEQAudioProcessor::getParameters() {
    return parameters;
}

juce::AudioProcessorValueTreeState::ParameterLayout GraphicEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GainBand1", 1), "32HzBand", 0.01f, 3.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GainBand2", 1), "63HzBand", 0.01f, 3.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GainBand3", 1), "125HzBand", 0.01f, 3.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GainBand4", 1), "250HzBand", 0.01f, 3.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GainBand5", 1), "500HzBand", 0.01f, 3.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GainBand6", 1), "1000HzBand", 0.01f, 3.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GainBand7", 1), "2000HzBand", 0.01f, 3.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GainBand8", 1), "4000HzBand", 0.01f, 3.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GainBand9", 1), "8000HzBand", 0.01f, 3.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GainBand10", 1), "16000HzBand", 0.01f, 3.0f, 1.0f));
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new GraphicEQAudioProcessor();
}
