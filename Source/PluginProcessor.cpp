/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DrumVisualizerAudioProcessor::DrumVisualizerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

DrumVisualizerAudioProcessor::~DrumVisualizerAudioProcessor()
{
}

//==============================================================================
const juce::String DrumVisualizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DrumVisualizerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DrumVisualizerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DrumVisualizerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DrumVisualizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DrumVisualizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DrumVisualizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DrumVisualizerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DrumVisualizerAudioProcessor::getProgramName (int index)
{
    return {};
}

void DrumVisualizerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DrumVisualizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Guardamos el sample rate para uso en funciones MIDI
    this->sampleRate = sampleRate;
}

void DrumVisualizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DrumVisualizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DrumVisualizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

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
    }
}

//==============================================================================
bool DrumVisualizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DrumVisualizerAudioProcessor::createEditor()
{
    return new DrumVisualizerAudioProcessorEditor (*this);
}

//==============================================================================
void DrumVisualizerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DrumVisualizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// Funciones para manejo de archivos MIDI

bool DrumVisualizerAudioProcessor::loadMidiFile(const juce::File& file)
{
    // Verifica que el archivo exista
    if (!file.existsAsFile())
    {
        juce::Logger::writeToLog("Error: El archivo no existe - " + file.getFullPathName());
        return false;
    }
    
    // Verifica la extensión del archivo
    juce::String extension = file.getFileExtension().toLowerCase();
    if (extension != ".mid" && extension != ".midi")
    {
        juce::Logger::writeToLog("Error: Formato de archivo no soportado - " + extension);
        return false;
    }
    
    // Intenta cargar el archivo MIDI
    juce::FileInputStream inputStream(file);
    if (!inputStream.openedOk())
    {
        juce::Logger::writeToLog("Error: No se pudo abrir el archivo - " + file.getFullPathName());
        return false;
    }
    
    // Lee el archivo MIDI
    midiFile.clear();
    if (!midiFile.readFrom(inputStream))
    {
        juce::Logger::writeToLog("Error: No se pudo leer el archivo MIDI - Archivo posiblemente dañado");
        return false;
    }
    
    // Si llegamos aquí, la carga fue exitosa
    loadedMidiFile = file;
    midiLoaded = true;
    
    // Log información del archivo cargado
    juce::Logger::writeToLog("MIDI cargado exitosamente:");
    juce::Logger::writeToLog("  Archivo: " + file.getFileName());
    juce::Logger::writeToLog("  Número de pistas: " + juce::String(getNumTracks()));
    juce::Logger::writeToLog("  Tempo: " + juce::String(getTempoFromMidi(), 2) + " BPM");
    juce::Logger::writeToLog("  Duración: " + juce::String(getLengthInSeconds(), 2) + " segundos");
    juce::Logger::writeToLog("  Rango de notas: " + juce::String(getLowestNote()) + " - " + juce::String(getHighestNote()));
    
    return true;
}

void DrumVisualizerAudioProcessor::clearMidiData()
{
    midiFile.clear();
    loadedMidiFile = juce::File();
    midiLoaded = false;
    juce::Logger::writeToLog("Datos MIDI limpiados");
}

bool DrumVisualizerAudioProcessor::hasMidiLoaded() const
{
    return midiLoaded;
}

juce::String DrumVisualizerAudioProcessor::getLoadedFileName() const
{
    if (midiLoaded)
        return loadedMidiFile.getFileName();
    return "Ningún archivo cargado";
}

int DrumVisualizerAudioProcessor::getNumTracks() const
{
    if (midiLoaded)
        return midiFile.getNumTracks();
    return 0;
}

double DrumVisualizerAudioProcessor::getTempoFromMidi() const
{
    if (!midiLoaded)
        return 120.0; // Tempo por defecto
        
    // Busca eventos de tempo en todas las pistas
    for (int track = 0; track < midiFile.getNumTracks(); ++track)
    {
        const auto* trackPtr = midiFile.getTrack(track);
        if (trackPtr != nullptr)
        {
            for (int i = 0; i < trackPtr->getNumEvents(); ++i)
            {
                const auto& event = trackPtr->getEventPointer(i)->message;
                if (event.isTempoMetaEvent())
                {
                    return event.getTempoSecondsPerQuarterNote() > 0 ? 
                           60.0 / event.getTempoSecondsPerQuarterNote() : 120.0;
                }
            }
        }
    }
    
    return 120.0; // Valor por defecto si no se encuentra tempo
}

double DrumVisualizerAudioProcessor::getLengthInSeconds() const
{
    if (!midiLoaded)
        return 0.0;
        
    return midiFile.getLastTimestamp();
}

// Funciones para obtener datos MIDI para el piano roll
const juce::MidiFile& DrumVisualizerAudioProcessor::getMidiFile() const
{
    return midiFile;
}

juce::Array<juce::MidiMessage> DrumVisualizerAudioProcessor::getAllNoteEvents() const
{
    juce::Array<juce::MidiMessage> noteEvents;
    
    if (!midiLoaded)
        return noteEvents;
    
    // Iterar sobre todas las pistas
    for (int track = 0; track < midiFile.getNumTracks(); ++track)
    {
        const auto* trackPtr = midiFile.getTrack(track);
        if (trackPtr != nullptr)
        {
            // Iterar sobre todos los eventos de la pista
            for (int i = 0; i < trackPtr->getNumEvents(); ++i)
            {
                const auto& event = trackPtr->getEventPointer(i)->message;
                
                // Solo agregar eventos de nota (Note On y Note Off)
                if (event.isNoteOn() || event.isNoteOff())
                {
                    noteEvents.add(event);
                }
            }
        }
    }
    
    return noteEvents;
}

int DrumVisualizerAudioProcessor::getLowestNote() const
{
    if (!midiLoaded)
        return 0;
    
    int lowestNote = 127; // Valor máximo MIDI
    bool foundNote = false;
    
    // Iterar sobre todas las pistas
    for (int track = 0; track < midiFile.getNumTracks(); ++track)
    {
        const auto* trackPtr = midiFile.getTrack(track);
        if (trackPtr != nullptr)
        {
            // Iterar sobre todos los eventos de la pista
            for (int i = 0; i < trackPtr->getNumEvents(); ++i)
            {
                const auto& event = trackPtr->getEventPointer(i)->message;
                
                // Solo considerar eventos de nota
                if (event.isNoteOn())
                {
                    foundNote = true;
                    lowestNote = std::min(lowestNote, event.getNoteNumber());
                }
            }
        }
    }
    
    return foundNote ? lowestNote : 0;
}

int DrumVisualizerAudioProcessor::getHighestNote() const
{
    if (!midiLoaded)
        return 127;
    
    int highestNote = 0; // Valor mínimo MIDI
    bool foundNote = false;
    
    // Iterar sobre todas las pistas
    for (int track = 0; track < midiFile.getNumTracks(); ++track)
    {
        const auto* trackPtr = midiFile.getTrack(track);
        if (trackPtr != nullptr)
        {
            // Iterar sobre todos los eventos de la pista
            for (int i = 0; i < trackPtr->getNumEvents(); ++i)
            {
                const auto& event = trackPtr->getEventPointer(i)->message;
                
                // Solo considerar eventos de nota
                if (event.isNoteOn())
                {
                    foundNote = true;
                    highestNote = std::max(highestNote, event.getNoteNumber());
                }
            }
        }
    }
    
    return foundNote ? highestNote : 127;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumVisualizerAudioProcessor();
}
