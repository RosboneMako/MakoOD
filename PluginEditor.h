/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/

struct t_KnobCoors {
    float x;
    float y;
    float sizex;
    float sizey;
};

//R1.00 Create a new LnF class based on Juces LnF class.
class MakoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    //R1.00 Let the user select a knob style.
    int MakoSliderKnobStyle = 3;
    float Kpts[32];
    juce::Path pathKnob;

    MakoLookAndFeel()
    {
        //R1.00 Define the Path points to make a knob (Style 3).
        Kpts[0] = -2.65325243300477f;
        Kpts[1] = 8.60001462363607f;
        Kpts[2] = 0.0f;
        Kpts[3] = 10.0f;
        Kpts[4] = 2.65277678639377f;
        Kpts[5] = 8.60016135439157f;
        Kpts[6] = 7.81826556234706f;
        Kpts[7] = 6.23495979109873f;
        Kpts[8] = 8.3778301945593f;
        Kpts[9] = 3.28815468479365f;
        Kpts[10] = 9.74931428347318f;
        Kpts[11] = -2.22505528067641f;
        Kpts[12] = 7.79431009355225f;
        Kpts[13] = -4.4998589050713f;
        Kpts[14] = 4.3390509473009f;
        Kpts[15] = -9.00958583269659f;
        Kpts[16] = 1.34161181197136f;
        Kpts[17] = -8.89944255254108f;
        Kpts[18] = -4.33855264588318f;
        Kpts[19] = -9.00982579958681f;
        Kpts[20] = -6.12133095297134f;
        Kpts[21] = -6.59767439058605f;
        Kpts[22] = -9.74919120703023f;
        Kpts[23] = -2.22559448434896f;
        Kpts[24] = -8.97486228392824f;
        Kpts[25] = .672195644527914f;
        Kpts[26] = -7.81861038843018f;
        Kpts[27] = 6.23452737534543f;
        Kpts[28] = -5.07025014121689f;
        Kpts[29] = 7.4358969536627f;
        Kpts[30] = -2.65325243300477f;
        Kpts[31] = 8.60001462363607f;

        //R1.00 Create the actual PATH for our KNOB style 3.
        pathKnob.startNewSubPath(Kpts[0], Kpts[1]);
        for (int t = 0; t < 32; t += 2)
        {
            pathKnob.lineTo(Kpts[t], Kpts[t + 1]);
        }
        pathKnob.closeSubPath();

        //R1.00 Recreate our points with smoothed corners.
        //pathKnob = pathKnob.createPathWithRoundedCorners(4.0f);
    }

    //R1.00 Override the Juce SLIDER drawing function so our code gets called instead of Juces code.
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& sld) override
    {
        //R1.00 Most of these are from JUCE demo code. Could be reduced if not used.
        //R1.00 Radius changed to -6.0f to reduce the final slider size.
        auto radius = (float)juce::jmin(width / 2, height / 2) - 6.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        //R1.00 Our Var defs.
        float sinA;
        float cosA;
        juce::ColourGradient ColGrad;
        int col;
              
        //1.00 Draw the KNOB face.
        ColGrad = juce::ColourGradient(juce::Colour(0xFF606060), 0.0f, y, juce::Colour(0xFF101010), 0.0f, y + height, false);
        g.setGradientFill(ColGrad);
        g.fillEllipse(rx, ry, rw, rw);       

        //R1.00 Draw shading around knob face.
        //g.setColour(juce::Colours::black);
        //g.drawEllipse(rx - 2.0f, ry - 2.0f, rw + 4.0f, rw + 4.0f, 1.0f);
        ColGrad = juce::ColourGradient(juce::Colour(0xFFFFFFFF), 0.0f, y, juce::Colour(0xFF606060), 0.0f, y + height, false);
        g.setGradientFill(ColGrad);
        g.drawEllipse(rx, ry, rw, rw, 2.0f);        

        //R1.00 Dont draw anymore objects if the control is disabled.
        if (sld.isEnabled() == false) return;

        //R1.00 Copy our predefined KNOB PATH, scale it, and then transform it to the centre position.
        //R1.00 The knob SIZE must be performed first. It is then ROTATED around its center. Then moved (TRANSLATED) to the screen knob position.
        juce::Path pK = pathKnob;
        pK.applyTransform(juce::AffineTransform::scale(radius / 11.0f).followedBy(juce::AffineTransform::rotation(angle).translated(centreX, centreY)));
        ColGrad = juce::ColourGradient(juce::Colour(0xFFC0C0C0), 0.0f, y, juce::Colour(0xFF000000), 0.0f, y + height, false);
        g.setGradientFill(ColGrad);
        g.strokePath(pK, juce::PathStrokeType(2.0f));
                
        //R1.00 Draw finger adjust indicator.
        sinA = std::sinf(angle) * radius;
        cosA = std::cosf(angle) * radius;
        g.setColour(juce::Colours::whitesmoke);
        g.drawLine(centreX + sinA * .75f, centreY - cosA * .75f, centreX + sinA, centreY - cosA, 4.0f);
                
    }
};



//R1.00 Add SLIDER listener. BUTTON or TIMER listeners also go here if needed. Must add ValueChanged overrides!
class MakoBiteAudioProcessorEditor  : public juce::AudioProcessorEditor , public juce::Slider::Listener, public juce::Timer //, public juce::Button::Listener , public juce::Timer
{
public:
    MakoBiteAudioProcessorEditor (MakoBiteAudioProcessor&);
    ~MakoBiteAudioProcessorEditor() override;

    void timerCallback() override;

    //R1.00 OUR override functions.
    void sliderValueChanged(juce::Slider* slider) override;

    //R1.00 Define an IMAGE object to hold our background image.
    //R1.00 The images are added in PROJUCER and embedded into our C++ Project.
    juce::Image imgBackground;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MakoBiteAudioProcessor& audioProcessor;

    //R1.00 Define a Look and Feel object for the class we created above.
    MakoLookAndFeel otherLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MakoBiteAudioProcessorEditor)

    void GUI_Init_Large_Slider(juce::Slider* slider, float Val, float Vmin, float Vmax, float Vinterval, juce::String Suffix);
    void GUI_Init_Small_Slider(juce::Slider* slider);

    //R1.00 Define our UI Juce Slider controls.
    int Knob_Cnt = 0;
    juce::Slider sldKnob[20];

    //R1.00 Define the coords and text for our knobs. Not JUCE related. 
    t_KnobCoors Knob_Pos[20] = {};
    juce::String Knob_Name[20] = {};
    void KNOB_DefinePosition(int t, float x, float y, float sizex, float sizey, juce::String name);

    //R1.00 Add some context sensitive help text. 
    //R1.00 The HelpString indexes MUST match the enumerated values e_Gain,e_NGate, etc.
    int ctrlHelp = 0;
    int ctrlHelpLast = 0;
    juce::Label labHelp;
    juce::String HelpString[20] =
    {
        "Adjust the volume for this effect. Careful not to clip.",
        "Reduce noise when not playing by turning down the gain.",
        "Adjust the lower voice of the OD. Set near 700Hz for typical OD.",
        "Adjust the higher voice of the OD. Set near 700Hz for typical OD.",
        "Adjust the OverDrive distortion.",
        "Boost the low mids after gain to fatten.",
        "Boost the highs before gain to crispen.",
        "Adjust the mix between clean and effect signals.",        
    };

    //R1.00 Label to show user we are clipping (Too loud). 
    juce::Label labClipping;
    bool STATE_Clip = false;


public:
    
    //R1.00 Define an 'enumerated' type list to make our SETTING and SLIDER code easier.
    //R1.00 Must match the audioProcessor values. Duplicated here just to make the 
    //R1.00 code easier to read and less likely to have errors.
    const int e_Gain = audioProcessor.e_Gain;
    const int e_NGate = audioProcessor.e_NGate;
    const int e_Low = audioProcessor.e_Low;
    const int e_High = audioProcessor.e_High;
    const int e_Drive = audioProcessor.e_Drive;
    const int e_EnhLow = audioProcessor.e_EnhLow;
    const int e_EnhHigh = audioProcessor.e_EnhHigh;
    const int e_Mix = audioProcessor.e_Mix;

    //R1.00 Define our SLIDER attachment variables.
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> ParAtt[10];
    
};
