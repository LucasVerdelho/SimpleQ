/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleQAudioProcessorEditor::SimpleQAudioProcessorEditor (SimpleQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto* comp : getComps())
    {
		addAndMakeVisible(comp);
	}


    setSize (600, 400);
}

SimpleQAudioProcessorEditor::~SimpleQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    // Area for the frequency response graph (top 1/3 of the bounds)
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    // We have 2/3 of vertical space left that we need to divide in 3:
    // The left 1/3 is for the low cut, the middle 1/3 is for the peak filter
    // and the right 1/3 is for the high cut
 
    // Area for the filter controls
    // Take the left 1/3 of the remaining bounds for the low cut
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    // Take the right 1/2 of the remaining bounds for the high cut 
    // (essentially 1/3 of the original bounds)
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

    // Set the bounds of the sliders
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.75));
    lowCutSlopeSlider.setBounds(lowCutArea);

    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.75));
    highCutSlopeSlider.setBounds(highCutArea);

    // Set the bounds of the peak filter controls (freq, gain, Q)
    // Take the top 1/3 of the remaining bounds for the frequency
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    // Take the top 1/2 of the remaining bounds for the gain
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    // The remaining bounds are for the Q
    peakQualitySlider.setBounds(bounds);


}

// Simplify the code by using a vector to store the components
std::vector<juce::Component*> SimpleQAudioProcessorEditor::getComps()
{
	return
    {
		&peakFreqSlider,
		&peakGainSlider,
		&peakQualitySlider,
		&lowCutFreqSlider,
		&highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider
	};
}
