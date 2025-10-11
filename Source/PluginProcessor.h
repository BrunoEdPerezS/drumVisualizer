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
class DrumVisualizerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DrumVisualizerAudioProcessor();
    ~DrumVisualizerAudioProcessor() override;

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

    //==============================================================================
    // Funciones para manejo de archivos MIDI
    bool loadMidiFile (const juce::File& file);
    void clearMidiData();
    bool hasMidiLoaded() const;
    juce::String getLoadedFileName() const;
    int getNumTracks() const;
    double getTempoFromMidi() const;
    double getLengthInSeconds() const;

    // Funciones para obtener datos MIDI para el piano roll
    const juce::MidiFile& getMidiFile() const;
    juce::Array<juce::MidiMessage> getAllNoteEvents() const;
    int getLowestNote() const;
    int getHighestNote() const;

private:
    //==============================================================================
    // Variables para manejo de archivos MIDI
    juce::MidiFile midiFile;
    juce::File loadedMidiFile;
    bool midiLoaded = false;
    double sampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumVisualizerAudioProcessor)
};
