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
class DrumVisualizerAudioProcessorEditor : public juce::AudioProcessorEditor, 
                                            public juce::TextEditor::Listener,
                                            public juce::ComboBox::Listener
{
public:
    DrumVisualizerAudioProcessorEditor (DrumVisualizerAudioProcessor&);
    ~DrumVisualizerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Listener overrides para el patr�n observer
    void textEditorTextChanged (juce::TextEditor& editor) override;
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;

    // Variables p�blicas para acceso desde otras clases
    int bpmVALUE = 120;
    float speedVALUE = 1.0f;
    juce::String timefigVALUE = "1/4";

private:
    // Widgets del primer contenedor (topFrame)
    juce::TextEditor textEditor; // Entrada de texto
    juce::TextButton exploreButton { "Explorar" }; // Bot�n "Explorar"
    juce::TextButton loadMidiButton { "Cargar MIDI" }; // Bot�n "Cargar MIDI"

    // Widgets del segundo contenedor (middleFrame)
    juce::Label bpmLabel; // Label para "BPM SET"
    juce::TextEditor bpmEditor; // Input de texto para BPM (solo acepta int)
    juce::Label speedLabel; // Label para "VELOCIDAD"
    juce::ComboBox speedComboBox; // Lista desplegable para Speed
    juce::TextButton playPauseButton { "PLAY/PAUSE" }; // Bot�n Play/Pause
    juce::TextButton stopButton { "STOP" }; // Bot�n Stop
    juce::Label timeFigLabel; // Label para "TIME FIGURE"
    juce::ComboBox timeFigComboBox; // Lista desplegable para Time Fig

    // M�todos privados para manejar cambios
    void updateBpmValue();
    void updateSpeedValue();
    void updateTimeFigValue();

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DrumVisualizerAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumVisualizerAudioProcessorEditor)
};
