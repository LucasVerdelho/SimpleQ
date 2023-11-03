/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleQAudioProcessor::SimpleQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SimpleQAudioProcessor::~SimpleQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize = samplesPerBlock;

    spec.numChannels = 1;

    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    updateFilters();


    leftChannelFifo.prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);

}

void SimpleQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
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

void SimpleQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
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


    updateFilters();

    // Prepare to get each channel's block of data
    juce::dsp::AudioBlock<float> block(buffer);

    // Get the left and right channels
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    // Create a context for each channel
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);   


    leftChannelFifo.update(buffer);
    rightChannelFifo.update(buffer);

}

//==============================================================================
bool SimpleQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleQAudioProcessor::createEditor()
{

    return new SimpleQAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);


}

//==============================================================================
void SimpleQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::MemoryOutputStream memoryos(destData, true);
    apvts.state.writeToStream(memoryos);
}

void SimpleQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        updateFilters();
    }

}


ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
	ChainSettings settings;

	settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
	settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
	settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
	settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
	settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
	settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
	settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());

	return settings;
}



Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate)
{
	return juce::
            dsp::
            IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                     chainSettings.peakFreq,
                                                     chainSettings.peakQuality,
                                                     juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
}



void SimpleQAudioProcessor::updatePeakFilter(const ChainSettings& chainSettings)
{
    auto peakCoefficients = makePeakFilter(chainSettings, getSampleRate());

    updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
}



void updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
	*old = *replacements;
}



void SimpleQAudioProcessor::updateLowCutFilters(const ChainSettings& chainSettings)
{
	auto lowcutCoefficients = makeLowCutFilter(chainSettings, getSampleRate());
	auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
	auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();

	updateCutFilter(leftLowCut, lowcutCoefficients, chainSettings.lowCutSlope);
	updateCutFilter(rightLowCut, lowcutCoefficients, chainSettings.lowCutSlope);
}



void SimpleQAudioProcessor::updateHighCutFilters(const ChainSettings& chainSettings)
{
	auto highcutCoefficients = makeHighCutFilter(chainSettings, getSampleRate());
	auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
	auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();

	updateCutFilter(leftHighCut, highcutCoefficients, chainSettings.highCutSlope);
	updateCutFilter(rightHighCut, highcutCoefficients, chainSettings.highCutSlope);
}


void SimpleQAudioProcessor::updateFilters()
{
	auto chainSettings = getChainSettings(apvts);
	updateLowCutFilters(chainSettings);
	updatePeakFilter(chainSettings);
	updateHighCutFilters(chainSettings);
}


juce::AudioProcessorValueTreeState::ParameterLayout SimpleQAudioProcessor::createParameterLayout()
{
    // Create a vector of parameters
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Add a low cut filter
	layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq", 
                                                           "LowCut Freq", 
                                                            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                            20.f));
    // Add a high cut filter
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
                                                           "HighCut Freq",
                                                            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                            20000.f));
    // Add a peak filter
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq",
                                                           "Peak Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           750.f));
    // Add a peak gain control
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain",
                                                           "Peak Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));
    // Add a peak quality control (Q)
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality",
                                                           "Peak Quality",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
                                                           1.f));

    // Create options for the filter type (ammount of db reduction per octave)
    juce::StringArray stringArray;
    for (int i = 0; i < 4; ++i)
    {
		juce::String str;
		str << (12 + i * 12);
		str << " db/Oct";
		stringArray.add(str);
	}

    // Add a low cut slope control
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope",
        												   "LowCut Slope",
        												   stringArray,
        												   0));

    // Add a high cut slope control
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope",
        												   "HighCut Slope",
        												   stringArray,
        												   0));


	return layout;
}






//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleQAudioProcessor();
}
