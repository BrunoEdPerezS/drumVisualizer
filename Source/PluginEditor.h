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

    // Listener overrides para el patrón observer
    void textEditorTextChanged (juce::TextEditor& editor) override;
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;

    // Variables públicas para acceso desde otras clases
    int bpmVALUE = 120;
    float speedVALUE = 1.0f;
    juce::String timefigVALUE = "1/4";

private:
    // Widgets del primer contenedor (topFrame)
    juce::TextEditor textEditor; // Entrada de texto
    juce::TextButton exploreButton { "Explorar" }; // Botón "Explorar"
    juce::TextButton loadMidiButton { "Cargar MIDI" }; // Botón "Cargar MIDI"

    // Widgets del segundo contenedor (middleFrame)
    juce::Label bpmLabel; // Label para "BPM SET"
    juce::TextEditor bpmEditor; // Input de texto para BPM (solo acepta int)
    juce::Label speedLabel; // Label para "VELOCIDAD"
    juce::ComboBox speedComboBox; // Lista desplegable para Speed
    juce::TextButton playPauseButton { "PLAY/PAUSE" }; // Botón Play/Pause
    juce::TextButton stopButton { "STOP" }; // Botón Stop
    juce::Label timeFigLabel; // Label para "TIME FIGURE"
    juce::ComboBox timeFigComboBox; // Lista desplegable para Time Fig

    // Variables para el piano roll
    juce::Rectangle<int> pianoRollArea;
    double timeZoom = 100.0; // Pixels per second
    int noteHeight = 10; // Altura de cada nota en pixels
    bool showPianoRoll = false;

    // Métodos privados para manejar cambios
    void updateBpmValue();
    void updateSpeedValue();
    void updateTimeFigValue();

    // Métodos para manejo de archivos MIDI
    void openFileChooser();
    void loadMidiFile();
    bool validateFilePath(const juce::String& path);
    void showMessage(const juce::String& title, const juce::String& message);
    void updateUIAfterMidiLoad();

    // Métodos para el piano roll
    void drawPianoRoll(juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawPianoKeys(juce::Graphics& g, const juce::Rectangle<int>& keyArea, int lowestNote, int highestNote);
    void drawMidiNotes(juce::Graphics& g, const juce::Rectangle<int>& noteArea, int lowestNote, int highestNote);
    void drawTimeGrid(juce::Graphics& g, const juce::Rectangle<int>& area);
    double timeToX(double timeInSeconds, const juce::Rectangle<int>& area) const;
    int noteToY(int noteNumber, int lowestNote, int highestNote, const juce::Rectangle<int>& area) const;
    juce::String getNoteNameFromNumber(int noteNumber) const;

    // Variables para manejo de archivos
    juce::String lastBrowsedDirectory;
    std::unique_ptr<juce::FileChooser> fileChooser;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DrumVisualizerAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumVisualizerAudioProcessorEditor)
};
