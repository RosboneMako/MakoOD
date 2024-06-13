/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MakoBiteAudioProcessorEditor::MakoBiteAudioProcessorEditor (MakoBiteAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    //R1.00 Create SLIDER ATTACHMENTS so our parameter vars get adjusted automatically for Get/Set states.
    ParAtt[e_Gain] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "gain", sldKnob[e_Gain]);
    ParAtt[e_NGate] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "ngate", sldKnob[e_NGate]);
    ParAtt[e_Low] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "low", sldKnob[e_Low]);
    ParAtt[e_High] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "high", sldKnob[e_High]);
    ParAtt[e_Drive] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "drive", sldKnob[e_Drive]);
    ParAtt[e_EnhLow] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "enhlow", sldKnob[e_EnhLow]);
    ParAtt[e_EnhHigh] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "enhhigh", sldKnob[e_EnhHigh]);
    ParAtt[e_Mix] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "mix", sldKnob[e_Mix]);
        
    //****************************************************************************************
    //R1.00 Add GUI CONTROLS
    //****************************************************************************************
    GUI_Init_Large_Slider(&sldKnob[e_Gain], audioProcessor.Setting[e_Gain], 0.0f, 2.0f, .01f, ""); 
    GUI_Init_Large_Slider(&sldKnob[e_NGate], audioProcessor.Setting[e_NGate], 0.0f, 1.0f, .01f, "");
    GUI_Init_Large_Slider(&sldKnob[e_Low], audioProcessor.Setting[e_Low], 100, 700, 25, " Hz");
    GUI_Init_Large_Slider(&sldKnob[e_High], audioProcessor.Setting[e_High], 700, 1800, 50, " Hz");
    GUI_Init_Large_Slider(&sldKnob[e_Drive], audioProcessor.Setting[e_Drive], 0.0f, 1.0f, .01f, "");
    GUI_Init_Large_Slider(&sldKnob[e_EnhLow], audioProcessor.Setting[e_EnhLow], 0.0f, 1.0f, .01f, "");
    GUI_Init_Large_Slider(&sldKnob[e_EnhHigh], audioProcessor.Setting[e_EnhHigh], 0.0f, 1.0f, .01f, "");
    GUI_Init_Large_Slider(&sldKnob[e_Mix], audioProcessor.Setting[e_Mix], 0.0f, 1.0f, .01f, "");

    //R1.00 Define the knob (slider) positions on the screen/UI.
    KNOB_DefinePosition(e_Gain,   10, 60, 90, 90, "Gain");
    KNOB_DefinePosition(e_Drive, 350, 60, 90, 90, "Drive");
    KNOB_DefinePosition(e_NGate, 115, 20, 70, 80, "Noise Gate");
    KNOB_DefinePosition(e_Low,   190, 20, 70, 80, "Low");
    KNOB_DefinePosition(e_High,  265, 20, 70, 80, "High");
    KNOB_DefinePosition(e_EnhLow,  115, 130, 70, 80, "Enh Low");
    KNOB_DefinePosition(e_EnhHigh, 190, 130, 70, 80, "Enh High");
    KNOB_DefinePosition(e_Mix,     265, 130, 70, 80, "Mix");
    
    //R1.00 Keep track of how many knobs we have setup for our loops later.
    //R1.00 Good programmers would use a LIST here instead of an array? But we are aiming for easy.
    Knob_Cnt = 8;

    //R1.00 Update the generic JUCE Look and Feel (Global colors) so drop down menu is the correct color. 
    getLookAndFeel().setColour(juce::DocumentWindow::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::DocumentWindow::textColourId, juce::Colour(255, 255, 255));
    getLookAndFeel().setColour(juce::DialogWindow::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(10, 100, 150));
    getLookAndFeel().setColour(juce::TextButton::buttonOnColourId, juce::Colour(10, 100, 150));
    getLookAndFeel().setColour(juce::TextButton::buttonColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::ComboBox::backgroundColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::ListBox::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::Label::backgroundColourId, juce::Colour(32, 32, 32));

    //R1.00 Assign our image var to the image PROJUCER built into our project. See MakoOD folder in solution window.
    imgBackground = juce::ImageCache::getFromMemory(BinaryData::makoodback01_jpg, BinaryData::makoodback01_jpgSize);

    //R2.00 Start our Timer so we can tell the user they are clipping. Could draw VU Meters here, etc.
    startTimerHz(2);  //R1.00 have our Timer get called twice per second.

    //R1.00 Create a label to let user know they are clipping.
    //R1.00 Color vars are AARRGGBB. Alpha, Red, Green, Blue. Alpha 00 = see thru.
    labClipping.setJustificationType(juce::Justification::centred);
    labClipping.setColour(juce::Label::backgroundColourId, juce::Colour(0x00000000)); //R1.00 We dont want to see this so make it see thru.
    labClipping.setColour(juce::Label::textColourId, juce::Colour(0xFF000000));       //R1.00 BLACK for OFF state.
    labClipping.setColour(juce::Label::outlineColourId, juce::Colour(0x00000000));    //R1.00 We dont want to see this so make it see thru.
    labClipping.setText("CLIPPING", juce::dontSendNotification);
    addAndMakeVisible(labClipping);

    //R1.00 Help Text! Must be LAST defined object to be blank at the start.
    labHelp.setJustificationType(juce::Justification::centred);
    labHelp.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF000000));
    labHelp.setColour(juce::Label::textColourId, juce::Colour(0xFFA0A0A0));
    labHelp.setColour(juce::Label::outlineColourId, juce::Colour(0xFF404040));
    addAndMakeVisible(labHelp);
    labHelp.setText("Mako OverDrive v1.0", juce::dontSendNotification);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    //R1.00 Set the window size LAST. Resize starts immediately.
    //R1.00 Last or none of your stuff will draw because it isnt defined yet.
    setSize(450, 250);
}

MakoBiteAudioProcessorEditor::~MakoBiteAudioProcessorEditor()
{
}

void MakoBiteAudioProcessorEditor::timerCallback()
{
    //R1.00 Check processor if audio is clipping.
    //R1.00 Track the Label stats so we are not redrawing the control twice a second.
    if (audioProcessor.AudioIsClipping)
    {
        audioProcessor.AudioIsClipping = false;
        STATE_Clip = true;
        labClipping.setColour(juce::Label::textColourId, juce::Colour(0xFFFFFF00));
    }
    else
    {
        if (STATE_Clip) 
            labClipping.setColour(juce::Label::textColourId, juce::Colour(0xFF000000));
        STATE_Clip = false;
    }
}

//==============================================================================
void MakoBiteAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    bool UseBackgroundImage = true;
    if (UseBackgroundImage)
    {
        //R1.00 We are using a saved image as our UI background.
        //R1.00 Slider controls are automatically drawn over by JUCE.
        g.drawImageAt(imgBackground, 0, 0);
    }
    else
    {
        //R1.00 OPTIONAL Background drawing code, if not using IMAGE.
        juce::ColourGradient ColGrad;
        ColGrad = juce::ColourGradient(juce::Colour(0xFF505050), 0.0f, 0.0f, juce::Colour(0xFF202020), 0.0f, 140.0f, false);
        g.setGradientFill(ColGrad);
        g.fillRect(0, 0, 450, 141);
        ColGrad = juce::ColourGradient(juce::Colour(0xFF202020), 0.0f, 140.0f, juce::Colour(0xFF404040), 0.0f, 280.0f, false);
        g.setGradientFill(ColGrad);
        g.fillRect(0, 140, 450, 140);

        //R1.00 OPTIONAL Header drawing code.
        g.setColour(juce::Colour(0xFF903030));
        for (int t = 0; t < Knob_Cnt; t++) g.fillRect(Knob_Pos[t].x, Knob_Pos[t].y - 15.0f, Knob_Pos[t].sizex, 15.0f);
        g.setFont(12.0f);
        g.setColour(juce::Colours::white);
        for (int t = 0; t < Knob_Cnt; t++) g.drawFittedText(Knob_Name[t], Knob_Pos[t].x, Knob_Pos[t].y - 15, Knob_Pos[t].sizex, 15, juce::Justification::centred, 1);
    }
}

void MakoBiteAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //R1.00 Draw all of the defined KNOBS.
    for (int t = 0; t < Knob_Cnt; t++) sldKnob[t].setBounds(Knob_Pos[t].x, Knob_Pos[t].y, Knob_Pos[t].sizex, Knob_Pos[t].sizey);

    labClipping.setBounds(360, 15, 70, 18);
    labHelp.setBounds(5, 220, 440, 18);
}

void MakoBiteAudioProcessorEditor::GUI_Init_Large_Slider(juce::Slider* slider, float Val, float Vmin, float Vmax, float Vinterval, juce::String Suffix)
{
    //R1.00 Setup the slider edit parameters.
    slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    slider->setTextValueSuffix(Suffix);
    slider->setRange(Vmin, Vmax, Vinterval);
    slider->setValue(Val);
    slider->addListener(this);
    addAndMakeVisible(slider);

    //R1.00 Override the default Juce drawing routines and use ours.
    //R1.00 This is how we have custom SLIDER controls.
    slider->setLookAndFeel(&otherLookAndFeel);

    slider->setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider->setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFFF4040));
    slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxHighlightColourId, juce::Colour(0xFFA04040));
    slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFF5085E8));
    slider->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::thumbColourId, juce::Colour(0xFF808080));
}

void MakoBiteAudioProcessorEditor::GUI_Init_Small_Slider(juce::Slider* slider)
{
    slider->setSliderStyle(juce::Slider::LinearHorizontal);
    slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 20);
    slider->setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFA0A0A0));
    slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF202020));
    slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxHighlightColourId, juce::Colour(0xFF600000));
    slider->setColour(juce::Slider::trackColourId, juce::Colour(0xFFC00000));
    slider->setColour(juce::Slider::backgroundColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::thumbColourId, juce::Colour(0xFF808080));
}

void MakoBiteAudioProcessorEditor::KNOB_DefinePosition(int idx, float x, float y, float sizex, float sizey, juce::String name)
{
    //R1.00 Define our knob positions so we can just loop thru and draw them all easily.
    Knob_Pos[idx].x = x;
    Knob_Pos[idx].y = y;
    Knob_Pos[idx].sizex = sizex;
    Knob_Pos[idx].sizey = sizey;
    Knob_Name[idx] = name;
}

void MakoBiteAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{    

    //R1.00 Check which slider has been adjusted.
    for (int t = 0; t < Knob_Cnt; t++)
    {
        if (slider == &sldKnob[t])
        {
            //R1.00 Update HELP bar with help for the SLIDER being adjusted.
            labHelp.setText(HelpString[t], juce::dontSendNotification);

            //R1.00 Update the actual processor variable being edited.
            audioProcessor.Setting[t] = float(sldKnob[t].getValue());

            //R1.00 Increment changed var to be sure, every change gets made, changed var is decremented in processor.
            audioProcessor.SettingsChanged += 1;

            //R1.00 We have captured the correct slider change, exit this function.
            return;
        }
    }
    
    //R1.00 OPTIONAL method for Sliders not using our CUSTOM LookAndFeel drawing functions.
    //if (slider == &jsP1_Gain)
    //{
    //    labHelp.setText("Adjust the final volume.", juce::dontSendNotification);
    //    audioProcessor.Pedal_Gain = float(jsP1_Gain.getValue());
    //    return;
    //}    
    
    return;
}
