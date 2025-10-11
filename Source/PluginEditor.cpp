/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DrumVisualizerAudioProcessorEditor::DrumVisualizerAudioProcessorEditor (DrumVisualizerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set the editor's size to the specified dimensions
    setSize (1200, 800);
    setResizable(false, false); // Allow resizing

    // Variable para almacenar la dirección del archivo
    juce::String DIRECCION_ARCHIVO = "Ingrese texto aquí";

    // Configurar y añadir los widgets al primer marco

    // Cuadro de texto para ingresar la dirección del archivo
    addAndMakeVisible(textEditor);
    textEditor.setText(DIRECCION_ARCHIVO);

    // Botón para explorar archivos
    addAndMakeVisible(exploreButton);
    exploreButton.setButtonText("Explorar");
    exploreButton.onClick = [] { juce::Logger::writeToLog("Botón Explorar presionado"); };

    // Botón para cargar archivos MIDI
    addAndMakeVisible(loadMidiButton);
    loadMidiButton.setButtonText("Cargar MIDI");
    loadMidiButton.onClick = [] { juce::Logger::writeToLog("Botón Cargar MIDI presionado"); };

    // Configurar y añadir los widgets al segundo marco (middleFrame)

    // Label para BPM
    addAndMakeVisible(bpmLabel);
    bpmLabel.setText("BPM SET", juce::dontSendNotification);
    bpmLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    bpmLabel.setJustificationType(juce::Justification::centred);

    // Input de texto para BPM (solo acepta enteros)
    addAndMakeVisible(bpmEditor);
    bpmEditor.setText("120");
    bpmEditor.setInputRestrictions(0, "0123456789"); // Solo números
    bpmEditor.addListener(this); // Registrar listener para cambios

    // Label para Speed
    addAndMakeVisible(speedLabel);
    speedLabel.setText("VELOCIDAD", juce::dontSendNotification);
    speedLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    speedLabel.setJustificationType(juce::Justification::centred);

    // ComboBox para Speed con opciones específicas
    addAndMakeVisible(speedComboBox);
    speedComboBox.addItem("0.5", 1);
    speedComboBox.addItem("0.75", 2);
    speedComboBox.addItem("1", 3);
    speedComboBox.addItem("1.25", 4);
    speedComboBox.addItem("1.5", 5);
    speedComboBox.addItem("2", 6);
    speedComboBox.setSelectedId(3); // Selecciona "1" por defecto
    speedComboBox.addListener(this); // Registrar listener para cambios

    // Botón Play/Pause
    addAndMakeVisible(playPauseButton);
    playPauseButton.onClick = [] { juce::Logger::writeToLog("Botón Play/Pause presionado"); };

    // Botón Stop
    addAndMakeVisible(stopButton);
    stopButton.onClick = [] { juce::Logger::writeToLog("Botón Stop presionado"); };

    // Label para Time Figure
    addAndMakeVisible(timeFigLabel);
    timeFigLabel.setText("TIME FIGURE", juce::dontSendNotification);
    timeFigLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    timeFigLabel.setJustificationType(juce::Justification::centred);

    // ComboBox para Time Fig con opciones específicas
    addAndMakeVisible(timeFigComboBox);
    timeFigComboBox.addItem("1/4", 1);
    timeFigComboBox.addItem("1/8", 2);
    timeFigComboBox.addItem("1/16", 3);
    timeFigComboBox.setSelectedId(1); // Selecciona "1/4" por defecto
    timeFigComboBox.addListener(this); // Registrar listener para cambios

    // Inicializar valores
    updateBpmValue();
    updateSpeedValue();
    updateTimeFigValue();
}

DrumVisualizerAudioProcessorEditor::~DrumVisualizerAudioProcessorEditor()
{
    // Desregistrar listeners
    bpmEditor.removeListener(this);
    speedComboBox.removeListener(this);
    timeFigComboBox.removeListener(this);
}

//==============================================================================
void DrumVisualizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Establece el color de fondo de la ventana
    g.fillAll (juce::Colour::fromString("#404040"));

    // Define los márgenes de separación
    const int verticalPadding = 10; // Espaciado vertical entre los elementos
    const int horizontalPadding = 20; // Espaciado horizontal con los bordes laterales

    //---------------- COLORES ---------------------
    // Define el color para los marcos
    const juce::Colour FRAMES_COLOR = juce::Colour(255, 255, 255); // Color blanco

    //---------------- ASPECTOS ---------------------
    // Define el radio para las esquinas redondeadas
    const float cornerRadius = 10.0f;


    // Obtiene el área disponible para dibujar y aplica los márgenes
    auto bounds = getLocalBounds().reduced(horizontalPadding, verticalPadding);

    // Crea el primer marco (topFrame) en la parte superior
    auto topFrame = bounds.removeFromTop(50); // Altura de 50 píxeles
    g.setColour(FRAMES_COLOR);
    g.fillRoundedRectangle(topFrame.toFloat(), cornerRadius); // Dibuja el marco con esquinas redondeadas

    bounds.removeFromTop(verticalPadding); // Añade un espaciado vertical debajo del primer marco

    // Crea el segundo marco (middleFrame) en el medio
    auto middleFrame = bounds.removeFromTop(50); // Altura de 50 píxeles
    g.setColour(FRAMES_COLOR);
    g.fillRoundedRectangle(middleFrame.toFloat(), cornerRadius); // Dibuja el marco con esquinas redondeadas

    bounds.removeFromTop(verticalPadding); // Añade un espaciado vertical debajo del segundo marco

    // Crea el tercer marco (bottomFrame) en la parte inferior
    auto bottomFrame = bounds; // Usa el área restante
    g.setColour(FRAMES_COLOR);
    g.fillRoundedRectangle(bottomFrame.toFloat(), cornerRadius); // Dibuja el marco con esquinas redondeadas
}

void DrumVisualizerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(30, 10); // Padding del contenedor

    // Espaciado entre widgets
    const int horizontalSpacing = 20; // Espaciado horizontal entre widgets
    const int verticalSpacing = 10;   // Espaciado vertical dentro del contenedor

    // Layout del primer marco (topContainer)
    auto topFrame = bounds.removeFromTop(50);

    // Configuración del input de texto
    auto textEditorWidth = static_cast<int>(topFrame.getWidth() * 0.75f); // 75% del ancho del contenedor
    auto buttonWidth = (topFrame.getWidth() - textEditorWidth - horizontalSpacing * 2) / 2; // Espacio restante dividido entre los botones

    textEditor.setBounds(topFrame.removeFromLeft(textEditorWidth).reduced(0, verticalSpacing));
    textEditor.setJustification(juce::Justification::centred); // Centrar texto verticalmente

    topFrame.removeFromLeft(horizontalSpacing); // Espaciado horizontal entre el input y el primer botón

    exploreButton.setBounds(topFrame.removeFromLeft(buttonWidth).reduced(0, verticalSpacing));

    topFrame.removeFromLeft(horizontalSpacing); // Espaciado horizontal entre los botones

    loadMidiButton.setBounds(topFrame.removeFromLeft(buttonWidth).reduced(0, verticalSpacing));

    // Espaciado vertical entre contenedores
    bounds.removeFromTop(verticalSpacing);

    // Layout del segundo marco (middleFrame)
    auto middleFrame = bounds.removeFromTop(50);

    // Calcular ancho de cada widget en el segundo contenedor (5 widgets en total)
    auto widgetWidth = (middleFrame.getWidth() - horizontalSpacing * 4) / 5; // 4 espacios entre 5 widgets

    // Contenedor BPM (dividido en Label + Input)
    auto bpmContainer = middleFrame.removeFromLeft(widgetWidth).reduced(0, verticalSpacing);
    auto bpmLabelWidth = bpmContainer.getWidth() / 2; // 50% para el label
    
    bpmLabel.setBounds(bpmContainer.removeFromLeft(bpmLabelWidth));
    bpmEditor.setBounds(bpmContainer); // El resto para el input

    middleFrame.removeFromLeft(horizontalSpacing); // Espaciado horizontal

    // Contenedor Speed (dividido en Label + ComboBox)
    auto speedContainer = middleFrame.removeFromLeft(widgetWidth).reduced(0, verticalSpacing);
    auto speedLabelWidth = speedContainer.getWidth() / 2; // 50% para el label
    
    speedLabel.setBounds(speedContainer.removeFromLeft(speedLabelWidth));
    speedComboBox.setBounds(speedContainer); // El resto para el combobox

    middleFrame.removeFromLeft(horizontalSpacing); // Espaciado horizontal

    // Play/Pause Button
    playPauseButton.setBounds(middleFrame.removeFromLeft(widgetWidth).reduced(0, verticalSpacing));

    middleFrame.removeFromLeft(horizontalSpacing); // Espaciado horizontal

    // Stop Button
    stopButton.setBounds(middleFrame.removeFromLeft(widgetWidth).reduced(0, verticalSpacing));

    middleFrame.removeFromLeft(horizontalSpacing); // Espaciado horizontal

    // Contenedor Time Figure (dividido en Label + ComboBox)
    auto timeFigContainer = middleFrame.removeFromLeft(widgetWidth).reduced(0, verticalSpacing);
    auto timeFigLabelWidth = timeFigContainer.getWidth() / 2; // 50% para el label
    
    timeFigLabel.setBounds(timeFigContainer.removeFromLeft(timeFigLabelWidth));
    timeFigComboBox.setBounds(timeFigContainer); // El resto para el combobox
}

//==============================================================================
// Implementación del patrón Observer
void DrumVisualizerAudioProcessorEditor::textEditorTextChanged(juce::TextEditor& editor)
{
    if (&editor == &bpmEditor)
    {
        updateBpmValue();
    }
}

void DrumVisualizerAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &speedComboBox)
    {
        updateSpeedValue();
    }
    else if (comboBoxThatHasChanged == &timeFigComboBox)
    {
        updateTimeFigValue();
    }
}

// Métodos privados para actualizar valores
void DrumVisualizerAudioProcessorEditor::updateBpmValue()
{
    auto text = bpmEditor.getText();
    if (!text.isEmpty())
    {
        bpmVALUE = text.getIntValue();
        juce::Logger::writeToLog("BPM actualizado: " + juce::String(bpmVALUE));
    }
}

void DrumVisualizerAudioProcessorEditor::updateSpeedValue()
{
    auto selectedText = speedComboBox.getText();
    if (!selectedText.isEmpty())
    {
        speedVALUE = selectedText.getFloatValue();
        juce::Logger::writeToLog("Speed actualizado: " + juce::String(speedVALUE));
    }
}

void DrumVisualizerAudioProcessorEditor::updateTimeFigValue()
{
    timefigVALUE = timeFigComboBox.getText();
    juce::Logger::writeToLog("Time Figure actualizado: " + timefigVALUE);
}
