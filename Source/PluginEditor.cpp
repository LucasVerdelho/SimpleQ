/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


void LookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, width);

    g.setColour(Colours::white);
    g.fillEllipse(bounds);

    g.setColour(Colours::darkgrey);
    g.drawEllipse(bounds, 1.f);

    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();

        Path p;

        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextBoxHeight() * 1.5);

        p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        g.setColour(Colour(0xff274560));
        g.fillRect(r);

        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }

}



void LookAndFeel::drawToggleButton(juce::Graphics& g,
                                   juce::ToggleButton& toggleButton,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    using namespace juce;

    // if the passed button is a powerbutton
    if (auto pb = dynamic_cast<PowerButton*>(&toggleButton))
    {
        Path powerButton;
        
        auto bounds = toggleButton.getLocalBounds();
        auto size = jmin(bounds.getWidth(), bounds.getHeight()) - 6;
        
        auto r = bounds.withSizeKeepingCentre(size, size).toFloat();
        
        float angle = 30.f * MathConstants<float>::pi / 180.f;
        
        size -= 6;
        
        powerButton.addCentredArc(r.getCentreX(),
                                  r.getCentreY(),
                                  size * 0.5f,
                                  size * 0.5f,
                                  0.f,
                                  angle,
                                  angle - MathConstants<float>::pi * 2.f,
                                  true);
        
        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());
        
        PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);
        auto color = toggleButton.getToggleState() ? Colour(0x66ff68a0) : Colour(0xffff68a0);
        g.setColour(color);
        g.strokePath(powerButton, pst);
        
        g.drawEllipse(r.toFloat(), 2.f);
    }

    else if (auto* ab = dynamic_cast<AnalyzerButton*>(&toggleButton))
    {
        auto color = toggleButton.getToggleState() ? Colour(0xffff68a0) : Colour(0x66ff68a0);
        g.setColour(color);

        auto bounds = toggleButton.getLocalBounds();
        auto insetRect = bounds.reduced(4.f);

        Path randomPath;

        Random r;
        randomPath.startNewSubPath(insetRect.getX(),
								   insetRect.getY() + insetRect.getHeight() * r.nextFloat());

        for (auto x = insetRect.getX() + 1; x < insetRect.getRight(); x += 2)
        {
            randomPath.lineTo(x,insetRect.getY() + insetRect.getHeight() * r.nextFloat());
        }

        g.strokePath(randomPath, PathStrokeType(2.f));
    }

}




void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    auto startAngle = degreesToRadians(180.f + 45.f);
    auto endAngle = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    //// Uncomment to see the bounds of the slider
    //g.setColour(Colours::red);
    //g.drawRect(getLocalBounds());
    //g.setColour(Colours::yellow);
    //g.drawRect(sliderBounds);

    getLookAndFeel().drawRotarySlider(g, 
                                      sliderBounds.getX(), 
                                      sliderBounds.getY(), 
                                      sliderBounds.getWidth(), 
                                      sliderBounds.getHeight(),
        		                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
        		                      startAngle, 
                                      endAngle, 
                                      *this);


    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;

    g.setColour(Colour(0xffcc537f));
    g.setFont(getTextHeight());

    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; i++)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos && pos <= 1.f);

        auto ang = jmap(pos, 0.f, 1.f, startAngle, endAngle);

        // Get the position of the label as if it's at the edge of the slider 
        // at the correct angle, and then add a small offset so that it's outside the slider
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);

        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidthFloat(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
   
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;

    juce::Rectangle<int> r;

    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;
}



juce::String RotarySliderWithLabels::getDisplayString() const
{
    if ( auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param) )
		return choiceParam->getCurrentChoiceName();

    juce::String str;
    bool addK = false;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
		float val = getValue();

        if (val > 999.f)
        {
			val /= 1000.f;
			addK = true;
		}

		str = juce::String(val, (addK ? 2 : 0));
	}
    else
    {
		jassertfalse; // Should never happen
	}

    if (suffix != "")
    {
        str << " ";
        if (addK)
			str << "k";

        str << suffix;
    }


    return str;
}





//==============================================================================

ResponseCurveComponent::ResponseCurveComponent(SimpleQAudioProcessor& p) :
    audioProcessor(p),
    //leftChannelFifo(&audioProcessor.leftChannelFifo)
    leftPathProducer(audioProcessor.leftChannelFifo),
    rightPathProducer(audioProcessor.rightChannelFifo)
{
	const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
		param->addListener(this);
	}

    updateChain();

	startTimerHz(60);
}



ResponseCurveComponent::~ResponseCurveComponent()
{
	const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
		param->removeListener(this);
	}
}



void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}


void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;

    while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0)
    {
        if (leftChannelFifo->getAudioBuffer(tempIncomingBuffer))
        {
            auto size = tempIncomingBuffer.getNumSamples();

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
                monoBuffer.getReadPointer(0, size),
                monoBuffer.getNumSamples() - size);

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                tempIncomingBuffer.getReadPointer(0, 0),
                size);


            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);

        }
    }

    const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();
    const auto binWidth = sampleRate / double(fftSize);

    while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
    {
        std::vector<float> fftData;
        if (leftChannelFFTDataGenerator.getFFTData(fftData))
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
        }
    }

    while (pathProducer.getNumPathsAvailable() > 0)
    {
        pathProducer.getPath(leftChannelFFTPath);
    }


}
void ResponseCurveComponent::timerCallback()
{
    if (shouldShowFFTAnalysis)
    {
        auto fftBounds = getAnalysisArea().toFloat();
        auto sampleRate = audioProcessor.getSampleRate();

        leftPathProducer.process(fftBounds, sampleRate);
        rightPathProducer.process(fftBounds, sampleRate);
    }

    if (parametersChanged.compareAndSetBool(false, true))
    {
        // Update the mono chain
        updateChain();
        // Signal a repaint
        //repaint();
    }

    repaint();
}



void ResponseCurveComponent::updateChain()
{
    auto chainSettings = getChainSettings(audioProcessor.apvts);

    monoChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    monoChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
    monoChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);


    auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
    updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);

    auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());

    updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
    updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);

}



void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;

    g.fillAll(Colour(0xff182a3a));

    g.drawImage(background, getLocalBounds().toFloat());

    auto responseArea = getAnalysisArea();

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

        if (!monoChain.isBypassed<ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);


        if ( !monoChain.isBypassed<ChainPositions::LowCut>() )
        {
            if (!lowCut.isBypassed<0>())
                mag *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

            if (!lowCut.isBypassed<1>())
                mag *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

            if (!lowCut.isBypassed<2>())
                mag *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

            if (!lowCut.isBypassed<3>())
                mag *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        }

        if (!monoChain.isBypassed<ChainPositions::HighCut>())
        {
            if (!highCut.isBypassed<0>())
                mag *= highCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

            if (!highCut.isBypassed<1>())
                mag *= highCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

            if (!highCut.isBypassed<2>())
                mag *= highCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

            if (!highCut.isBypassed<3>())
                mag *= highCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }


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


    // Frequency Spectrum Analyzer
    if (shouldShowFFTAnalysis)
    {
        // Left Channel
        auto leftChannelFFTPath = leftPathProducer.getPath();
        leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));

        g.setColour(Colours::lightskyblue);
        g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));

        // Right Channel
        auto rightChannelFFTPath = rightPathProducer.getPath();
        rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));

        g.setColour(Colours::seagreen);
        g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));

    }


    // Draw a box around the response curve
    //g.setColour(Colour(0xff376186));
    g.setColour(Colour(0xffff68a0)); //Alternative colour
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);

    // Draw the response curve
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));

}



void ResponseCurveComponent::resized()
{
	using namespace juce;
    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);

    Graphics g(background);

    Array<float> freqs
    {
        20,/*30,40,*/50,100,
        200,/*300,400,*/500,1000,
        2000,/*3000,4000,*/5000,10000,
        20000
	};

    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width  = renderArea.getWidth();


    g.setColour(Colour(0xff376186));
    //g.setColour(Colour(0xffff68a0)); //Alternative colour
    Array<float> xs;
    for (auto f : freqs)
    {
		auto normX = mapFromLog10(f, 20.f, 20000.f);
		xs.add(left + width * normX);
	}

    g.setColour(Colour(0x66376186)); // Dimmer Colour
    //g.setColour(Colour(0xffff68a0)); //Alternative colour
    for (auto x : xs)
    {
        g.drawVerticalLine(x, top, bottom);
    }

    Array<float> gain
    {
        -24, -12, 0, 12, 24
    };

    for (auto gdB : gain)
    {
        auto y = jmap(gdB, -24.f, 24.f, float(bottom), float(top));
        g.setColour(gdB == 0.f ? Colour(0xffff68a0) : Colour(0x66376186)); // Dimmer Colour
        g.drawHorizontalLine(y, left, right);
    }


    g.setColour(Colours::white);
    const int fontHeight = 10;
    g.setFont(fontHeight);

    for (int i = 0; i < freqs.size(); i++)
    {
        auto f = freqs[i];
        auto x = xs[i];

        bool addK = false;

        String str;
        if (f > 999.f)
        {
            addK = true;
            f /= 1000.f;
        }

        str << f;
        if (addK)
			str << "k";
        str << "Hz";

		auto textWidth = g.getCurrentFont().getStringWidth(str);

		Rectangle<int> r;
		r.setSize(textWidth, fontHeight);
		r.setCentre(x, 0);
        auto y = getBottom() - fontHeight;
		r.setY(y);

        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }


    for (auto gdB : gain)
    {
        // Gain text
        auto y = jmap(gdB, -24.f, 24.f, float(bottom), float(top));

        String str;
        if (gdB > 0)
            str << "+";
        str << gdB;

        auto textWidth = g.getCurrentFont().getStringWidth(str);
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(getWidth() - textWidth);
        r.setCentre(r.getCentreX(), y);
        g.setColour(gdB == 0.f ? Colour(0xffff68a0) : Colours::white);
		g.drawFittedText(str, r, juce::Justification::centred, 1);


        // Frequency spectrum analysis text
        str.clear();
        str << (gdB - 24.f);

        r.setX(1);
        textWidth = g.getCurrentFont().getStringWidth(str);
        r.setSize(textWidth, fontHeight);
        g.setColour(Colours::wheat);
        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }


}


juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();

    bounds.removeFromTop(4);
    bounds.removeFromBottom(12);
    bounds.removeFromLeft(18);
    bounds.removeFromRight(18);


    return bounds;
}



juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
	auto bounds = getRenderArea();

	bounds.removeFromTop(4);
    bounds.removeFromBottom(4);

	return bounds;
}





//==============================================================================
SimpleQAudioProcessorEditor::SimpleQAudioProcessorEditor (SimpleQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
    peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
    peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
    lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
    lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
    highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "dB/Oct"),

    responseCurveComponent(audioProcessor),
    peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),

    lowCutBypassButtonAttachment(audioProcessor.apvts, "LowCut Bypassed", lowCutBypassButton),
    peakBypassButtonAttachment(audioProcessor.apvts, "Peak Bypassed", peakBypassButton),
    highCutBypassButtonAttachment(audioProcessor.apvts, "HighCut Bypassed", highCutBypassButton),
    analyzerEnabledButtonAttachment(audioProcessor.apvts, "Analyzer Enabled", analyzerEnabledButton)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.


    peakFreqSlider.labels.add({ 0.f, "20Hz" });
    peakFreqSlider.labels.add({ 1.f, "20kHz" });

    peakGainSlider.labels.add({ 0.f, "-24dB" });
    peakGainSlider.labels.add({ 1.f, "+24dB" });

    peakQualitySlider.labels.add({ 0.f, "0.1" });
    peakQualitySlider.labels.add({ 1.f, "10.0" });

    lowCutFreqSlider.labels.add({ 0.f, "20Hz" });
    lowCutFreqSlider.labels.add({ 1.f, "20kHz" });

    highCutFreqSlider.labels.add({ 0.f, "20Hz" });
    highCutFreqSlider.labels.add({ 1.f, "20kHz" });

    lowCutSlopeSlider.labels.add({ 0.f, "12" });
    lowCutSlopeSlider.labels.add({ 1.f, "48" });

    highCutSlopeSlider.labels.add({ 0.f, "12" });
    highCutSlopeSlider.labels.add({ 1.f, "48" });




    for (auto* comp : getComps())
    {
		addAndMakeVisible(comp);
	}

    peakBypassButton.setLookAndFeel(&lnf);
    lowCutBypassButton.setLookAndFeel(&lnf);
    highCutBypassButton.setLookAndFeel(&lnf);
    analyzerEnabledButton.setLookAndFeel(&lnf);



    auto safePtr = juce::Component::SafePointer<SimpleQAudioProcessorEditor>(this);
    peakBypassButton.onClick = [safePtr]()
        {
            if (auto* comp = safePtr.getComponent())
            {
                auto bypassed = comp->peakBypassButton.getToggleState();

                comp->peakFreqSlider.setEnabled(!bypassed);
                comp->peakGainSlider.setEnabled(!bypassed);
                comp->peakQualitySlider.setEnabled(!bypassed);
            }
        };


    lowCutBypassButton.onClick = [safePtr]()
        {
            if (auto* comp = safePtr.getComponent())
            {
                auto bypassed = comp->lowCutBypassButton.getToggleState();

                comp->lowCutFreqSlider.setEnabled(!bypassed);
                comp->lowCutSlopeSlider.setEnabled(!bypassed);
            }
        };

    highCutBypassButton.onClick = [safePtr]()
        {
            if (auto* comp = safePtr.getComponent())
            {
                auto bypassed = comp->highCutBypassButton.getToggleState();

                comp->highCutFreqSlider.setEnabled(!bypassed);
                comp->highCutSlopeSlider.setEnabled(!bypassed);
            }
        };

    analyzerEnabledButton.onClick = [safePtr]()
        {
            if (auto* comp = safePtr.getComponent())
            {
                auto enabled = comp->analyzerEnabledButton.getToggleState();
                comp->responseCurveComponent.toggleAnalysisEnablement(enabled);
            }
        };








    setSize (600, 480);
}

SimpleQAudioProcessorEditor::~SimpleQAudioProcessorEditor()
{
    peakBypassButton.setLookAndFeel(nullptr);
    lowCutBypassButton.setLookAndFeel(nullptr);
    highCutBypassButton.setLookAndFeel(nullptr);
    analyzerEnabledButton.setLookAndFeel(nullptr);
}

//==============================================================================
void SimpleQAudioProcessorEditor::paint(juce::Graphics& g)
{
    using namespace  juce;

    g.fillAll(Colour(0xff182a3a));

}

void SimpleQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();

    auto analyzerEnabledArea = bounds.removeFromTop(25);
    analyzerEnabledArea.setWidth(100);
    analyzerEnabledArea.setX(5);
    analyzerEnabledArea.removeFromTop(2);

    analyzerEnabledButton.setBounds(analyzerEnabledArea);

    bounds.removeFromTop(5);


    // The ratio of the bounds that we want to use for the frequency response graph
    float hRatio = 0.33f;
    // Area for the frequency response graph (top 1/3 of the bounds)
    auto responseArea = bounds.removeFromTop(static_cast<int>(bounds.getHeight() * hRatio));
    responseCurveComponent.setBounds(responseArea);

    bounds.removeFromTop(5);

    // We have 2/3 of vertical space left that we need to divide in 3:
    // The left 1/3 is for the low cut, the middle 1/3 is for the peak filter
    // and the right 1/3 is for the high cut

    // Area for the filter controls
    // Take the left 1/3 of the remaining bounds for the low cut
    auto lowCutArea = bounds.removeFromLeft(static_cast<int>(bounds.getWidth() * 0.33));
    // Take the right 1/2 of the remaining bounds for the high cut 
    // (essentially 1/3 of the original bounds)
    auto highCutArea = bounds.removeFromRight(static_cast<int>(bounds.getWidth() * 0.5));


    // Set the bounds of the sliders and the buttons
    lowCutBypassButton.setBounds(lowCutArea.removeFromTop(25));
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(static_cast<int>(lowCutArea.getHeight() * 0.75)));
    lowCutSlopeSlider.setBounds(lowCutArea);

    highCutBypassButton.setBounds(highCutArea.removeFromTop(25));
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(static_cast<int>(highCutArea.getHeight() * 0.75)));
    highCutSlopeSlider.setBounds(highCutArea);

    // Bypass button for the peak filter
    peakBypassButton.setBounds(bounds.removeFromTop(25));
    // Set the bounds of the peak filter controls (freq, gain, Q)
    // Take the top 1/3 of the remaining bounds for the frequency
    peakFreqSlider.setBounds(bounds.removeFromTop(static_cast<int>(bounds.getHeight() * 0.33)));
    // Take the top 1/2 of the remaining bounds for the gain
    peakGainSlider.setBounds(bounds.removeFromTop(static_cast<int>(bounds.getHeight() * 0.5)));
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
        &highCutSlopeSlider,
        &responseCurveComponent,
        &lowCutBypassButton,
        &peakBypassButton,
        &highCutBypassButton,
        &analyzerEnabledButton






	};
}
