/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "cmath"              //R1.00 Added library.

//==============================================================================
MakoBiteAudioProcessor::MakoBiteAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
    ),

    //R1.00 Added for VALUE TREE. JUCE object that holds our parameter settings.
    parameters(*this, nullptr, "PARAMETERS", {
      std::make_unique<juce::AudioParameterFloat>("gain","Gain", .0f, 2.0f, 1.0f),
      std::make_unique<juce::AudioParameterFloat>("ngate","N Gate", .0f, 1.0f, .0f),
      std::make_unique<juce::AudioParameterInt>("low","Low", 100, 700, 350),
      std::make_unique<juce::AudioParameterInt>("high","High", 700, 1800, 1250),
      std::make_unique<juce::AudioParameterFloat>("drive","Drive", .0f, 1.0f, .2f),
      std::make_unique<juce::AudioParameterFloat>("enhlow","Enhlow", .0f, 1.0f, .0f),
      std::make_unique<juce::AudioParameterFloat>("enhhigh","Enhhigh", .0f, 1.0f, .0f),
      std::make_unique<juce::AudioParameterFloat>("mix","Mix", .0f, 1.0f, 1.0f),
    }

    )

#endif
{
   
}

MakoBiteAudioProcessor::~MakoBiteAudioProcessor()
{
}

//==============================================================================
const juce::String MakoBiteAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MakoBiteAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MakoBiteAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MakoBiteAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MakoBiteAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MakoBiteAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MakoBiteAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MakoBiteAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MakoBiteAudioProcessor::getProgramName (int index)
{
    return {};
}

void MakoBiteAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MakoBiteAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    //R1.00 Get our Sample Rate for filter calculations.
    //R1.00 192k max here. Probably should not do this.
    SampleRate = MakoBiteAudioProcessor::getSampleRate();
    if (SampleRate < 21000) SampleRate = 48000;
    if (192000 < SampleRate) SampleRate = 48000;

    //R1.00 Define our ENHANCE filters. These do not change, so calc once here.
    //R1.00 Our other filters change, so they are done in Settings_Update.
    Filter_BP_Coeffs(18.0f, 450, .707f, &makoF_OD_EnhLow);
    Filter_BP_Coeffs(18.0f, 1350, .707f, &makoF_OD_EnhHigh);

    //R1.00 Calc our OD low+high filters.
    Settings_Update(true);
}

void MakoBiteAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MakoBiteAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MakoBiteAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    //R1.00 Our defined variables.
    float tS;

    //R1.00 Handle any settings changes made in the Editor. Should only be small changes, so do not force all. 
    //R1.00 SettingsChanged will grow as more settings change. This is not a TRUE/FLASE situation.
    if (0 < SettingsChanged) Settings_Update(false);

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
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
        for (int samp = 0; samp < buffer.getNumSamples(); samp++)
        {

            tS = buffer.getSample(channel, samp);  //R1.00 Get the current sample and put it in tS. 
            tS = MakoOD_ProcessAudio(tS, channel); //R1.00 Apply our OD and Noise Gate to the sample. 
            channelData[samp] = tS;                //R1.00 Write our modified sample back into the sample buffer.
        }
    }
}

//==============================================================================
bool MakoBiteAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MakoBiteAudioProcessor::createEditor()
{
    return new MakoBiteAudioProcessorEditor (*this);
}


//==============================================================================
void MakoBiteAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    //R1.00 Save our parameters to file/DAW.
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);

}

void MakoBiteAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
        
    //R1.00 Read our parameters from file/DAW.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));

    //R1.00 Force our settings to update.
    Setting[e_Gain] = makoGetParmValue_float("gain");
    Setting[e_NGate] = makoGetParmValue_float("ngate");
    Setting[e_Low] = makoGetParmValue_int("low");
    Setting[e_High] = makoGetParmValue_int("high");
    Setting[e_Drive] = makoGetParmValue_float("drive");
    Setting[e_EnhLow] = makoGetParmValue_float("enhlow");
    Setting[e_EnhHigh] = makoGetParmValue_float("enhhigh");
    Setting[e_Mix] = makoGetParmValue_float("mix");

    //R1.00 ALL of our settings have changed. Force all settings to be recalculated.
    Settings_Update(true);

}

int MakoBiteAudioProcessor::makoGetParmValue_int(juce::String Pstring)
{
    //R1.00 Read a parameter and force it to a certain type if it exists.
    auto parm = parameters.getRawParameterValue(Pstring);
    if (parm != NULL)
        return int(parm->load());
    else
        return 0;
}

float MakoBiteAudioProcessor::makoGetParmValue_float(juce::String Pstring)
{
    //R1.00 Read a parameter and force it to a certain type if it exists.
    auto parm = parameters.getRawParameterValue(Pstring);
    if (parm != NULL)
        return float(parm->load());
    else
        return 0.0f;
}

float MakoBiteAudioProcessor::makoNoiseGate(float tSample, int channel)
{
    //R2.00 Create a volume envelope based on Signal Average.
    Pedal_NGate_Fac[channel] = Signal_AVG[channel] * 10000.0f * (1.1f - Setting[e_NGate]);
    if (1.0f < Pedal_NGate_Fac[channel]) Pedal_NGate_Fac[channel] = 1.0f;

    //R1.00 Apply our vol envelope to the audio data sample.
    return tSample * Pedal_NGate_Fac[channel];
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MakoBiteAudioProcessor();
}


float MakoBiteAudioProcessor::Filter_Calc_BiQuad(float tSample, int channel, tp_filter* fn)
{
    //R1.00 This applies an audio filter to our sample data. Coeffs are precalc'd. 
    float tS = tSample;

    fn->xn0[channel] = tS;
    tS = fn->a0 * fn->xn0[channel] + fn->a1 * fn->xn1[channel] + fn->a2 * fn->xn2[channel] - fn->b1 * fn->yn1[channel] - fn->b2 * fn->yn2[channel];
    fn->xn2[channel] = fn->xn1[channel]; fn->xn1[channel] = fn->xn0[channel]; fn->yn2[channel] = fn->yn1[channel]; fn->yn1[channel] = tS;

    return tS;
}

void MakoBiteAudioProcessor::Filter_BP_Coeffs(float Gain_dB, float Fc, float Q, tp_filter* fn)
{
    //R1.00 Second order parametric/peaking boost filter with constant-Q
    float K = pi2 * (Fc * .5f) / SampleRate;
    float K2 = K * K;
    float V0 = pow(10.0, Gain_dB / 20.0);

    float a = 1.0f + (V0 * K) / Q + K2;
    float b = 2.0f * (K2 - 1.0f);
    float g = 1.0f - (V0 * K) / Q + K2;
    float d = 1.0f - K / Q + K2;
    float dd = 1.0f / (1.0f + K / Q + K2);

    fn->a0 = a * dd;
    fn->a1 = b * dd;
    fn->a2 = g * dd;
    fn->b1 = b * dd;
    fn->b2 = d * dd;
    fn->c0 = 1.0f;
    fn->d0 = 0.0f;
}

void MakoBiteAudioProcessor::Filter_LP_Coeffs(float fc, tp_filter* fn)
{
    //R1.00 Second order LOW PASS filter. 
    float c = 1.0f / (tanf(pi * fc / SampleRate));
    fn->a0 = 1.0f / (1.0f + sqrt2 * c + (c * c));
    fn->a1 = 2.0f * fn->a0;
    fn->a2 = fn->a0;
    fn->b1 = 2.0f * fn->a0 * (1.0f - (c * c));
    fn->b2 = fn->a0 * (1.0f - sqrt2 * c + (c * c));
}

void MakoBiteAudioProcessor::Filter_HP_Coeffs(float fc, tp_filter* fn)
{
    //F1.00 Second order butterworth High Pass.
    float c = tanf(pi * fc / SampleRate);
    fn->a0 = 1.0f / (1.0f + sqrt2 * c + (c * c));
    fn->a1 = -2.0f * fn->a0;
    fn->a2 = fn->a0;
    fn->b1 = 2.0f * fn->a0 * ((c * c) - 1.0f);
    fn->b2 = fn->a0 * (1.0f - sqrt2 * c + (c * c));
}


float MakoBiteAudioProcessor::MakoOD_ProcessAudio(float tSample, int channel)
{
    float tS_Enh;

    //R1.00 Calc Low Frequency Band Pass filter. 
    float tS = Filter_Calc_BiQuad(tSample, channel, &makoF_OD_Low);

    //R1.00 Noise gate. Helps to Remove some Highs before testing Noise.
    //R1.00 So perform this action after the LOW filter.
    if (0.0f < Setting[e_NGate])
    {
        //R1.00 Track our Input Signal Average (Absolute vals).
        Signal_AVG[channel] = (Signal_AVG[channel] * .995) + (abs(tS) * .005);

        //R1.00 Apply Noise gate.
        tS = makoNoiseGate(tS, channel);
    }

    //R1.00 Enhance the high freqs a little.
    if (0.0f < Setting[e_EnhHigh])
    {
        tS_Enh = Filter_Calc_BiQuad(tS, channel, &makoF_OD_EnhHigh);
        tS += tanhf(tS_Enh * Setting[e_EnhHigh]);
    }

    tS = Filter_Calc_BiQuad(tS, channel, &makoF_OD_High);

    //R1.00 Make a copy of the cleanish filtered signal. Includes EnhHigh.
    float tS2 = tS * .25f;

    //R1.00 Apply gain. mix of clean and ODHT.
    tS = tanhf(tS * (.01f + (Setting[e_Drive] * Setting[e_Drive]) * 10.0f));
        
    //R1.00 Apply Clean to OD blend.
    tS = ((1.0f - Setting[e_Mix]) * tS2) + (Setting[e_Mix] * tS);

    //R1.00 Reduce gain, tanhf pushes the signal to the limits (-1,1).
    //R1.00 We could have a 1.0f + 1.0f situation, so reduce more then 50% (25% used here).
    tS *= .25f;

    //R1.00 Enhance the LOW/MID freqs a little.
    if (0.0f < Setting[e_EnhLow])
    {
        tS_Enh = Filter_Calc_BiQuad(tS, channel, &makoF_OD_EnhLow);
        tS += tanhf(tS_Enh * Setting[e_EnhLow]);
    }

    //R1.00 Volume/Gain adjust.
    tS = tS * Setting[e_Gain];

    //R1.00 Clip the signal to just below -1/1 so the audio engine does not crash. 
    //R1.00 Need a var here to let user know they are clipping. For now we will assume
    //R1.00 they will hear the crackle in the sound.
    if (tS < -.9999f)
    {
        tS = -.999f;
        AudioIsClipping = true;
    }
    if (.9999f < tS)
    {
        tS = .999f;
        AudioIsClipping = true;
    }

    return tS;
}

void MakoBiteAudioProcessor::Settings_Update(bool ForceAll)
{
    //R1.00 Here we verify our settings have not changed. If they did, update them.
    //R1.00 FORCEALL is set when massive changes have happened like the program start or PRESET update from DAW.
    //R1.00 We do these changes here in the Processor, because we dont want to change values as they are being used.
    //R1.00 We dont want the editor modifying things and getting weird results.   

    //R1.00 Update our Filters. Gain, Drive, Mix, NGate, etc do not need recalc unless you are smoothing the changes.
    if ((Setting[e_Low] != Setting_Last[e_Low]) || ForceAll)
    {
        Setting_Last[e_Low] = Setting[e_Low];
        Filter_BP_Coeffs(18.0f, Setting[e_Low], .707f, &makoF_OD_Low);
    }
    if ((Setting[e_High] != Setting_Last[e_High]) || ForceAll)
    {
        Setting_Last[e_High] = Setting[e_High];
        Filter_BP_Coeffs(18.0f, Setting[e_High], .707f, &makoF_OD_High);
    }
    

    //R1.00 Reduce our SettingChanged flag and limit to 0 (No changes). 
    SettingsChanged -= 1;
    if (SettingsChanged < 0) SettingsChanged = 0;
}

