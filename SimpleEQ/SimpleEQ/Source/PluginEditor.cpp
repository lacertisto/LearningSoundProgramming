/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), 
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

    for (juce::Component* comp : getComps())
	{
		addAndMakeVisible(comp);
    }

    setSize (600, 400);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
	using namespace juce;

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);

	Rectangle<int> bounds = getLocalBounds();
	Rectangle<int> responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33f);
	int w = responseArea.getWidth();

	CutFilter& lowCut = monoChain.get<ChainPositions::LowCut>();
	CutFilter& highCut = monoChain.get<ChainPositions::HighCut>();
	Filter& peak = monoChain.get<ChainPositions::Peak>();

	double sampleRate = audioProcessor.getSampleRate();

	std::vector<double> mags;

	mags.resize(w);

	for (int i =0; i < w; ++i)
	{
		double mag = 1.f;
		double freq = mapToLog10(static_cast<double>(i) / static_cast<double>(w), 20.0, 20000.0);

		if (! monoChain.isBypassed<ChainPositions::Peak>())
		{
			mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}

		if (!lowCut.isBypassed<0>())
		{
			mag *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!lowCut.isBypassed<1>())
		{
			mag *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!lowCut.isBypassed<2>())
		{
			mag *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!lowCut.isBypassed<3>())
		{
			mag *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}

		if (!highCut.isBypassed<0>())
		{
			mag *= highCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!highCut.isBypassed<1>())
		{
			mag *= highCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!highCut.isBypassed<2>())
		{
			mag *= highCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!highCut.isBypassed<3>())
		{
			mag *= highCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}

		mags[i] = Decibels::gainToDecibels(mag);
	}

	Path responseCurve;
	const double outputMin = responseArea.getBottom();
	const double outputMax = responseArea.getY();
	auto map = [&outputMin, &outputMax](double input)
	{
		return jmap(input, -24.0, 24.0, outputMin, outputMax); 
	};
	responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

	for (size_t i = 1; i < mags.size(); ++i)
	{
		responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
	}

	g.setColour(Colours::orange);
	g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);

	g.setColour(Colours::white);
	g.strokePath(responseCurve, PathStrokeType(2.f));
}

void SimpleEQAudioProcessorEditor::resized()
{
	// This is generally where you'll want to lay out the positions of any
	// subcomponents in your editor..

	juce::Rectangle<int> bounds = getLocalBounds();
	juce::Rectangle<int> responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33f);
	juce::Rectangle<int> lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33f);
	juce::Rectangle<int> highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5f); //returns last third of the display

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5f));
	lowCutSlopeSlider.setBounds(lowCutArea);
	highCutFreqSlider.setBounds(highCutArea.removeFromTop(lowCutArea.getHeight() * 0.5f));
	highCutSlopeSlider.setBounds(highCutArea);

    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33f));
	peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5f));
	peakQualitySlider.setBounds(bounds);
}

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
	return {&peakFreqSlider, &peakGainSlider, &peakQualitySlider, &lowCutFreqSlider, &highCutFreqSlider, &lowCutSlopeSlider, &highCutSlopeSlider};
}