/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class MakoBiteAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    MakoBiteAudioProcessor();
    ~MakoBiteAudioProcessor() override;

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

    //R1.00 Create a Parameter VALUETREE object to track our settings for DAW/FILE.
    juce::AudioProcessorValueTreeState parameters;                           
    
    //R1.00 Create a flag to let editor know we are clipping and show user.
    bool AudioIsClipping = false;

    //R1.00 Our public variables.
    int SettingsChanged = 0;
    int SettingsType = 0;
    float Setting[20] = {};       //R1.00 Actual Setting value.
    float Setting_Last[20] = {};  //R1.00 Last set value. Used to know when a VAR has been changed.

    //R1.00 Define arrays to store our NOISE GATE gain value and the AVERAGE signal level.
    float Pedal_NGate_Fac[2] = {};
    float Signal_AVG[2] = {};
    
    //R1.00 Define an 'enumerated' type list to make our SETTING and SLIDER code easier.
    //R1.00 Any of our custom SLIDERs you add should have a value added here.
    const int e_Gain = 0;
    const int e_NGate = 1;
    const int e_Low = 2;
    const int e_High = 3;
    const int e_Drive = 4;
    const int e_EnhLow = 5;
    const int e_EnhHigh = 6;
    const int e_Mix = 7;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MakoBiteAudioProcessor)

    //R1.00 Add some funcs to make parameters easier to deal with.
    int makoGetParmValue_int(juce::String Pstring);
    float makoGetParmValue_float(juce::String Pstring);

    //R1.00 The actual funcs that do the audio work.
    float makoNoiseGate(float tSample, int channel);
    float MakoOD_ProcessAudio(float tSample, int channel);

    //R1.00 Some Constants. SampleRate is updated at runtime in PrepareToPlay code. 
    const float pi = 3.14159265f;
    const float pi2 = 6.2831853f;
    const float sqrt2 = 1.4142135f;
    float SampleRate = 48000.0f;     //R1.00 Default value.

    //R1.00 OUR FILTER VARIABLES
    struct tp_coeffs {
        float a0;
        float a1;
        float a2;
        float b1;
        float b2;
        float c0;
        float d0;
    };

    struct tp_filter {
        float a0;
        float a1;
        float a2;
        float b1;
        float b2;
        float c0;
        float d0;
        float xn0[2];
        float xn1[2];
        float xn2[2];
        float yn1[2];
        float yn2[2];
        float offset[2];
    };

    //R1.00 FILTERS
    float Filter_Calc_BiQuad(float tSample, int channel, tp_filter* fn);
    void Filter_BP_Coeffs(float Gain_dB, float Fc, float Q, tp_filter* fn);
    void Filter_LP_Coeffs(float fc, tp_filter* fn);
    void Filter_HP_Coeffs(float fc, tp_filter* fn);

    //R1.00 Define our filters. 
    tp_filter makoF_OD_Low = {};
    tp_filter makoF_OD_High = {};
    tp_filter makoF_OD_EnhHigh = {};
    tp_filter makoF_OD_EnhLow = {};    

    //R1.00 Handle any paramater changes.
    void Settings_Update(bool ForceAll);

};
