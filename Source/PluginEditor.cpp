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
void SimpleQAudioProcessorEditor::paint(juce::Graphics& g)
{
    using namespace juce;

    g.fillAll(Colour(0xff182a3a));

    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    auto w = responseArea.getWidth();

    auto& lowCut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highCut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> mags;

    mags.resize(w);

    for (int i = 0; i < w; i++)
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

        if ( ! monoChain.isBypassed<ChainPositions::Peak>() )
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if ( !lowCut.isBypassed<0>() )
			mag *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
		if ( !lowCut.isBypassed<1>() )
            mag *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if ( !lowCut.isBypassed<2>() )
            mag *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if ( !lowCut.isBypassed<3>() )
            mag *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);


        if ( !highCut.isBypassed<0>() )
            mag *= highCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if ( !highCut.isBypassed<1>() )
            mag *= highCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if ( !highCut.isBypassed<2>() )
            mag *= highCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if ( !highCut.isBypassed<3>() )
            mag *= highCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        mags[i] = Decibels::gainToDecibels(mag);
    }

    Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        // -24 dB to +24 dB for the response curve to fit the peak filter
		return jmap(input, -24.0, 24.0, outputMin, outputMax);
	};

    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for (size_t i = 1; i < mags.size(); i++)
    {
		responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
	}

    // Draw a box around the response curve
    g.setColour(Colours::orange);
	g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);

    // Draw the response curve
	g.setColour(Colours::white);
	g.strokePath(responseCurve, PathStrokeType(2.f));
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



void SimpleQAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue)
{
	parametersChanged.set(true);
}

void SimpleQAudioProcessorEditor::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
		// Update the mono chain
        // Signal a repaint
	}
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
