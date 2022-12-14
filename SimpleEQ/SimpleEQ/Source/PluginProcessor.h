/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

enum ChainPositions
{
	LowCut,
	Peak,
	HighCut
};

struct ChainSettings
{
	float peakFreq{0}, peakGainInDecibels{0}, peakQuality{1.f};
	float lowCutFreq{0}, highCutFreq{0};
	int lowCutSlope{Slope::Slope_12}, highCutSlope{Slope::Slope_12};
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//alias for use of dsp namespace in project to eliminate usage of nested namespaces and template definitions
using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter,Filter,Filter,Filter>;
using MonoChain = juce::dsp::ProcessorChain<CutFilter,Filter,CutFilter>;
using Coefficients = Filter::CoefficientsPtr;

//==============================================================================
/**
*/
class SimpleEQAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts{
		*this,
		nullptr,
		"Parameters", 
        createParameterLayout()
	};

private:
    //using aliases declare two mono channels to represent stereo
    MonoChain leftChain, rightChain;

    void updatePeakFilter(const ChainSettings& chainSettings);
	static void updateCoefficients(Coefficients& old, const Coefficients& replacements);
    void updateLowCutFilters(const ChainSettings& chainSettings);
	void updateHighCutFilters(const ChainSettings& chainSettings);
    void updateFilters();

    template <int Index, typename ChainType, typename CoefficientType>
	void updateCutParams(ChainType& chain, const CoefficientType& coefficients)
	{
		SimpleEQAudioProcessor::updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
		chain.template setBypassed<Index>(false);
	}

	template <typename ChainType, typename CoefficientType, typename Slope>
	void updateCutFilter(ChainType& chain, CoefficientType& cutCoefficients, const Slope& lowCutSlope)
	{
		chain.template setBypassed<0>(true);
		chain.template setBypassed<1>(true);
		chain.template setBypassed<2>(true);
		chain.template setBypassed<3>(true);

		switch (lowCutSlope)
		{
			case Slope_48:
			{
				updateCutParams<3>(chain, cutCoefficients);
				__fallthrough;
			}
			case Slope_36:
			{
				updateCutParams<2>(chain, cutCoefficients);
				__fallthrough;
			}
			case Slope_24:
			{
				updateCutParams<1>(chain, cutCoefficients);
				__fallthrough;
			}
			case Slope_12:
			{
				updateCutParams<0>(chain, cutCoefficients);
				__fallthrough;
			}
			default:
				break;
		}
	}

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
