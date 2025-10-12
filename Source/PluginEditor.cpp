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
    juce::String DIRECCION_ARCHIVO = "Ingrese la ruta del archivo MIDI.";

    // Configurar y añadir los widgets al primer marco

    // Cuadro de texto para ingresar la dirección del archivo
    addAndMakeVisible(textEditor);
    textEditor.setText(DIRECCION_ARCHIVO);
    textEditor.addListener(this); // Añadir listener para cambios de texto

    // Botón para explorar archivos
    addAndMakeVisible(exploreButton);
    exploreButton.setButtonText("Explorar");
    exploreButton.onClick = [this] { openFileChooser(); };

    // Botón para cargar archivos MIDI
    addAndMakeVisible(loadMidiButton);
    loadMidiButton.setButtonText("Cargar MIDI");
    loadMidiButton.onClick = [this] { loadMidiFile(); };
    
    // Inicialmente deshabilitar el botón si el texto está vacío o es el placeholder
    loadMidiButton.setEnabled(false);

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
    playPauseButton.onClick = [this] { 
        if (isPlaying) 
            pausePlayback(); 
        else 
            startPlayback(); 
    };

    // Botón Stop
    addAndMakeVisible(stopButton);
    stopButton.onClick = [this] { stopPlayback(); };

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

    // Inicializar directorio por defecto para explorar archivos
    lastBrowsedDirectory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getFullPathName();
    
    // Configurar la línea objetivo al 25% del ancho (las notas vendrán de la derecha)
    targetLineX = static_cast<int>(getWidth() * 0.25f);
}

DrumVisualizerAudioProcessorEditor::~DrumVisualizerAudioProcessorEditor()
{
    // Parar el timer si está corriendo
    stopTimer();
    
    // Desregistrar listeners
    textEditor.removeListener(this);
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

    // Dibujar el piano roll si hay MIDI cargado
    if (audioProcessor.hasMidiLoaded())
    {
        // Área del piano roll con padding de 10 píxeles en todos los bordes
        pianoRollArea = bottomFrame.reduced(10);
        drawSynthesiaPianoRoll(g, pianoRollArea);
    }
    else
    {
        // Mostrar mensaje cuando no hay MIDI cargado
        g.setColour(juce::Colours::grey);
        g.setFont(juce::Font(juce::FontOptions(16.0f)));
        g.drawText("Carga un archivo MIDI para ver el piano roll", 
                   bottomFrame, 
                   juce::Justification::centred, 
                   true);
    }
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
// Timer callback para animación
void DrumVisualizerAudioProcessorEditor::timerCallback()
{
    if (isPlaying && audioProcessor.hasMidiLoaded())
    {
        // Calcular delta time
        juce::int64 currentSystemTime = juce::Time::getMillisecondCounter();
        if (lastUpdateTime == 0)
            lastUpdateTime = currentSystemTime;
        
        double deltaTime = (currentSystemTime - lastUpdateTime) / 1000.0; // Convertir a segundos
        lastUpdateTime = currentSystemTime;
        
        // Actualizar tiempo actual basado en BPM y velocidad
        double scrollSpeed = getScrollSpeed();
        currentTime += deltaTime * scrollSpeed;
        
        // Verificar si hemos llegado al final del MIDI
        double totalLength = audioProcessor.getLengthInSeconds();
        if (currentTime >= totalLength)
        {
            stopPlayback();
            resetToBeginning();
        }
        
        // Redibujar para mostrar la animación
        repaint();
    }
}

//==============================================================================
// Implementación del patrón Observer
void DrumVisualizerAudioProcessorEditor::textEditorTextChanged(juce::TextEditor& editor)
{
    if (&editor == &bpmEditor)
    {
        updateBpmValue();
    }
    else if (&editor == &textEditor)
    {
        // Validar ruta del archivo MIDI y habilitar/deshabilitar botón
        juce::String currentText = textEditor.getText();
        bool isValidPath = validateFilePath(currentText);
        loadMidiButton.setEnabled(isValidPath);
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
        int newBpm = text.getIntValue();
        if (newBpm > 0 && newBpm <= 300) // Validar rango razonable
        {
            bpmVALUE = newBpm;
            juce::Logger::writeToLog("BPM actualizado: " + juce::String(bpmVALUE));
            
            // Si está reproduciendo, la velocidad de scroll se actualizará automáticamente
            if (isPlaying)
            {
                juce::Logger::writeToLog("Velocidad de scroll actualizada por cambio de BPM");
            }
        }
        else
        {
            // Restaurar valor anterior si es inválido
            bpmEditor.setText(juce::String(bpmVALUE));
        }
    }
}

void DrumVisualizerAudioProcessorEditor::updateSpeedValue()
{
    auto selectedText = speedComboBox.getText();
    if (!selectedText.isEmpty())
    {
        float newSpeed = selectedText.getFloatValue();
        if (newSpeed > 0 && newSpeed <= 5.0) // Validar rango razonable
        {
            speedVALUE = newSpeed;
            juce::Logger::writeToLog("Speed actualizado: " + juce::String(speedVALUE));
            
            // Si está reproduciendo, la velocidad de scroll se actualizará automáticamente
            if (isPlaying)
            {
                juce::Logger::writeToLog("Velocidad de scroll actualizada por cambio de Speed");
            }
        }
    }
}

void DrumVisualizerAudioProcessorEditor::updateTimeFigValue()
{
    timefigVALUE = timeFigComboBox.getText();
    juce::Logger::writeToLog("Time Figure actualizado: " + timefigVALUE);
}

//==============================================================================
// Métodos para manejo de archivos MIDI

void DrumVisualizerAudioProcessorEditor::openFileChooser()
{
    // Crear el file chooser con filtros para archivos MIDI
    fileChooser = std::make_unique<juce::FileChooser>("Seleccionar archivo MIDI", 
                                                       juce::File(lastBrowsedDirectory), 
                                                       "*.mid;*.midi");
    
    // Configurar flags para el file chooser
    auto chooserFlags = juce::FileBrowserComponent::openMode | 
                        juce::FileBrowserComponent::canSelectFiles;
    
    // Abrir el file chooser de forma asíncrona
    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file != juce::File{})
        {
            // Actualizar la ruta en el textEditor
            textEditor.setText(file.getFullPathName());
            
            // Recordar el directorio para la próxima vez
            lastBrowsedDirectory = file.getParentDirectory().getFullPathName();
            
            // Habilitar el botón de carga
            loadMidiButton.setEnabled(true);
            
            juce::Logger::writeToLog("Archivo seleccionado: " + file.getFullPathName());
        }
    });
}

void DrumVisualizerAudioProcessorEditor::loadMidiFile()
{
    // Obtener la ruta del textEditor
    juce::String filePath = textEditor.getText().trim();
    
    // Validar que no esté vacía
    if (filePath.isEmpty() || filePath == "Ingrese la ruta del archivo MIDI")
    {
        showMessage("Error", "Por favor, ingresa una ruta o usa 'Explorar'.");
        return;
    }
    
    // Crear el objeto File
    juce::File midiFile(filePath);
    
    // Validar que el archivo exista
    if (!midiFile.existsAsFile())
    {
        showMessage("Error", "No se encontró el archivo en la ruta indicada:\n" + filePath);
        return;
    }
    
    // Validar extensión
    juce::String extension = midiFile.getFileExtension().toLowerCase();
    if (extension != ".mid" && extension != ".midi")
    {
        showMessage("Error", "Formato no soportado. Selecciona un archivo .mid o .midi.\nExtensión detectada: " + extension);
        return;
    }
    
    // Intentar cargar el archivo
    bool loadSuccess = audioProcessor.loadMidiFile(midiFile);
    
    if (loadSuccess)
    {
        showMessage("Carga exitosa", "MIDI cargado correctamente:\n" + midiFile.getFileName());
        updateUIAfterMidiLoad();
    }
    else
    {
        showMessage("Error", "No fue posible cargar el MIDI.");
    }
}

bool DrumVisualizerAudioProcessorEditor::validateFilePath(const juce::String& path)
{
    // Verificar que no esté vacío y no sea el texto placeholder
    if (path.isEmpty() || path == "Ingrese la ruta del archivo MIDI aquí")
        return false;
        
    // Verificar que tenga una extensión válida
    juce::String lowerPath = path.toLowerCase();
    return lowerPath.endsWith(".mid") || lowerPath.endsWith(".midi");
}

void DrumVisualizerAudioProcessorEditor::showMessage(const juce::String& title, const juce::String& message)
{
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        title,
        message,
        "OK"
    );
}

void DrumVisualizerAudioProcessorEditor::updateUIAfterMidiLoad()
{
    // Si el archivo MIDI tiene información de tempo, actualizar el BPM
    if (audioProcessor.hasMidiLoaded())
    {
        double midiTempo = audioProcessor.getTempoFromMidi();
        if (midiTempo > 0 && midiTempo != 120.0) // Solo actualizar si es diferente del default
        {
            bpmEditor.setText(juce::String(static_cast<int>(midiTempo)));
            updateBpmValue();
        }
        
        // Log información del archivo cargado para debug
        double totalLength = audioProcessor.getLengthInSeconds();
        int totalTracks = audioProcessor.getNumTracks();
        juce::Logger::writeToLog("MIDI cargado - Duración: " + juce::String(totalLength, 2) + "s, Pistas: " + juce::String(totalTracks));
        
        // Resetear la posición de reproducción
        resetToBeginning();
        repaint(); // Redibujar para mostrar el piano roll
    }
}

//==============================================================================
// Control de reproducción

void DrumVisualizerAudioProcessorEditor::startPlayback()
{
    if (!audioProcessor.hasMidiLoaded())
        return;
        
    isPlaying = true;
    playPauseButton.setButtonText("PAUSE");
    
    // Inicializar variables de tiempo
    lastUpdateTime = juce::Time::getMillisecondCounter();
    playbackStartTime = currentTime;
    
    // Iniciar el timer con 60 FPS para animación fluida
    startTimer(16); // ~60 FPS (1000ms / 60 = 16.67ms)
    
    juce::Logger::writeToLog("Reproducción iniciada desde tiempo: " + juce::String(currentTime, 2) + "s");
}

void DrumVisualizerAudioProcessorEditor::pausePlayback()
{
    isPlaying = false;
    playPauseButton.setButtonText("PLAY");
    stopTimer();
    
    juce::Logger::writeToLog("Reproducción pausada en tiempo: " + juce::String(currentTime, 2) + "s");
}

void DrumVisualizerAudioProcessorEditor::stopPlayback()
{
    isPlaying = false;
    playPauseButton.setButtonText("PLAY");
    stopTimer();
    resetToBeginning();
    
    juce::Logger::writeToLog("Reproducción detenida y reiniciada");
}

void DrumVisualizerAudioProcessorEditor::resetToBeginning()
{
    currentTime = 0.0;
    lastUpdateTime = 0;
    repaint();
}

//==============================================================================
// Cálculo de velocidad de scroll

double DrumVisualizerAudioProcessorEditor::getScrollSpeed() const
{
    // La velocidad base se basa en el BPM
    // A 120 BPM, queremos una velocidad de scroll de 1.0 (tiempo real)
    // El speedComboBox multiplica esta velocidad base
    double baseBeatRate = bpmVALUE / 120.0; // Normalizar contra 120 BPM
    return baseBeatRate * speedVALUE;
}

//==============================================================================
// Métodos para el piano roll estilo Synthesia

void DrumVisualizerAudioProcessorEditor::drawSynthesiaPianoRoll(juce::Graphics& g, const juce::Rectangle<int>& area)
{
    if (!audioProcessor.hasMidiLoaded())
        return;

    // Obtener el rango de notas del MIDI
    int lowestNote = audioProcessor.getLowestNote();
    int highestNote = audioProcessor.getHighestNote();
    
    // Asegurar un rango mínimo visible
    if (highestNote - lowestNote < 24) // Menos de 2 octavas
    {
        int center = (lowestNote + highestNote) / 2;
        lowestNote = center - 12;
        highestNote = center + 12;
    }
    
    // Clamp to MIDI range
    lowestNote = std::max(0, lowestNote - 2);
    highestNote = std::min(127, highestNote + 2);

    // Dividir el área: teclas del piano a la izquierda, notas a la derecha
    const int keyWidth = 80;
    auto workingArea = area; // Copia mutable del área
    auto keyArea = workingArea.removeFromLeft(keyWidth);
    auto noteArea = workingArea;

    // Calcular posición de la línea objetivo
    targetLineX = keyArea.getRight() + static_cast<int>(noteArea.getWidth() * 0.25f);

    // Dibujar las teclas del piano
    drawPianoKeys(g, keyArea, lowestNote, highestNote);
    
    // Dibujar fondo del área de notas
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRect(noteArea);
    
    // Dibujar escala de tiempo
    drawTimeScale(g, noteArea);
    
    // Dibujar las notas MIDI animadas
    drawAnimatedMidiNotes(g, noteArea, lowestNote, highestNote);
    
    // Dibujar línea objetivo (donde "caen" las notas)
    drawTargetLine(g, noteArea);
}

void DrumVisualizerAudioProcessorEditor::drawPianoKeys(juce::Graphics& g, const juce::Rectangle<int>& keyArea, int lowestNote, int highestNote)
{
    const int numNotes = highestNote - lowestNote + 1;
    const float keyHeight = (float)keyArea.getHeight() / (float)numNotes;
    
    for (int i = 0; i < numNotes; ++i)
    {
        int noteNumber = highestNote - i; // Dibujar desde arriba hacia abajo
        float y = (float)keyArea.getY() + (float)i * keyHeight;
        juce::Rectangle<float> keyRect((float)keyArea.getX(), y, (float)keyArea.getWidth(), keyHeight);
        
        // Determinar si es tecla blanca o negra
        int noteInOctave = noteNumber % 12;
        bool isBlackKey = (noteInOctave == 1 || noteInOctave == 3 || noteInOctave == 6 || 
                          noteInOctave == 8 || noteInOctave == 10);
        
        // Color de la tecla
        if (isBlackKey)
        {
            g.setColour(juce::Colour(0xff1a1a1a));
        }
        else
        {
            g.setColour(juce::Colours::white);
        }
        
        g.fillRect(keyRect);
        
        // Borde de la tecla
        g.setColour(juce::Colours::grey);
        g.drawRect(keyRect, 1.0f);
        
        // Dibujar nombre de la nota para teclas blancas o notas C
        if (!isBlackKey || noteInOctave == 0)
        {
            g.setColour(isBlackKey ? juce::Colours::white : juce::Colours::black);
            g.setFont(juce::Font(juce::FontOptions(10.0f)));
            g.drawText(getNoteNameFromNumber(noteNumber), 
                      keyRect.reduced(2), 
                      juce::Justification::centredLeft, 
                      true);
        }
    }
}

void DrumVisualizerAudioProcessorEditor::drawAnimatedMidiNotes(juce::Graphics& g, const juce::Rectangle<int>& noteArea, int lowestNote, int highestNote)
{
    if (!audioProcessor.hasMidiLoaded())
        return;

    const auto& midiFile = audioProcessor.getMidiFile();
    
    // Ampliar significativamente la ventana de tiempo visible
    double windowStart = currentTime - 1.0; // Mostrar notas 1s antes del tiempo actual
    double windowEnd = currentTime + 12.0; // Ventana de scroll mucho más amplia (12 segundos adelante)
    
    // Si no estamos reproduciendo, mostrar todo el archivo desde el inicio
    if (!isPlaying)
    {
        windowStart = 0.0;
        windowEnd = std::max(12.0, audioProcessor.getLengthInSeconds());
    }
    
    // Iterar sobre todas las pistas
    for (int track = 0; track < midiFile.getNumTracks(); ++track)
    {
        const auto* trackPtr = midiFile.getTrack(track);
        if (trackPtr != nullptr)
        {
            // Iterar sobre todos los eventos de la pista
            for (int i = 0; i < trackPtr->getNumEvents(); ++i)
            {
                const auto& eventHolder = *trackPtr->getEventPointer(i);
                const auto& event = eventHolder.message;
                
                // Solo dibujar eventos Note On con velocidad > 0
                if (event.isNoteOn() && event.getVelocity() > 0)
                {
                    int noteNumber = event.getNoteNumber();
                    double noteTime = event.getTimeStamp();
                    
                    // Solo dibujar notas que están en la ventana de tiempo visible
                    if (noteTime >= windowStart && noteTime <= windowEnd &&
                        noteNumber >= lowestNote && noteNumber <= highestNote)
                    {
                        float velocity = (float)event.getVelocity() / 127.0f;
                        
                        // Calcular posición X animada
                        float x = (float)timeToAnimatedX(noteTime, noteArea, windowEnd - currentTime);
                        float y = (float)noteToY(noteNumber, lowestNote, highestNote, noteArea);
                        
                        // Solo dibujar si la nota está dentro del área visible
                        if (x >= noteArea.getX() - 25 && x <= noteArea.getRight() + 25)
                        {
                            // Tamaño de la nota
                            float noteWidth = 20.0f; // Más ancho para mejor visibilidad
                            float height = (float)noteArea.getHeight() / (float)(highestNote - lowestNote + 1) * 0.8f;
                            
                            // Color basado en la velocidad con efecto de brillo
                            juce::Colour baseColor = juce::Colour::fromHSV(0.6f, 0.9f, 0.4f + velocity * 0.6f, 1.0f);
                            
                            // Efecto de resplandor para notas que están cerca de la línea objetivo
                            float distanceToTarget = std::abs(x - targetLineX);
                            float maxGlowDistance = 50.0f;
                            if (distanceToTarget < maxGlowDistance)
                            {
                                float glowIntensity = 1.0f - (distanceToTarget / maxGlowDistance);
                                baseColor = baseColor.brighter(glowIntensity * 0.3f);
                            }
                            
                            // Dibujar nota con efecto de resplandor
                            g.setColour(baseColor.withAlpha(0.3f));
                            g.fillRoundedRectangle(x - 2, y - 2, noteWidth + 4, height + 4, 4.0f);
                            
                            g.setColour(baseColor);
                            g.fillRoundedRectangle(x, y, noteWidth, height, 3.0f);
                            
                            // Borde brillante
                            g.setColour(baseColor.brighter(0.4f));
                            g.drawRoundedRectangle(x, y, noteWidth, height, 3.0f, 2.0f);
                        }
                    }
                }
            }
        }
    }
}

void DrumVisualizerAudioProcessorEditor::drawTargetLine(juce::Graphics& g, const juce::Rectangle<int>& area)
{
    // Dibujar línea objetivo vertical brillante
    int lineX = targetLineX;
    
    // Efecto de resplandor
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.fillRect(lineX - 3, area.getY(), 7, area.getHeight());
    
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.fillRect(lineX - 2, area.getY(), 5, area.getHeight());
    
    // Línea principal
    g.setColour(juce::Colours::white);
    g.fillRect(lineX - 1, area.getY(), 3, area.getHeight());
}

void DrumVisualizerAudioProcessorEditor::drawTimeScale(juce::Graphics& g, const juce::Rectangle<int>& area)
{
    // Dibujar líneas de tiempo futuras que se acercan
    g.setColour(juce::Colour(0xff333333));
    
    double scrollSpeed = getScrollSpeed();
    double beatsPerSecond = (bpmVALUE / 60.0) * scrollSpeed;
    double beatInterval = 1.0 / beatsPerSecond; // Segundos por beat
    
    // Usar ventana ampliada para mostrar más beats
    double windowWidth = isPlaying ? 12.0 : std::max(12.0, audioProcessor.getLengthInSeconds());
    
    // Dibujar líneas de compás que se mueven
    for (double futureTime = currentTime; futureTime <= currentTime + windowWidth; futureTime += beatInterval)
    {
        float x = (float)timeToAnimatedX(futureTime, area, windowWidth);
        if (x >= area.getX() && x <= area.getRight())
        {
            g.drawVerticalLine((int)x, (float)area.getY(), (float)area.getBottom());
        }
    }
}

//==============================================================================
// Funciones de conversión para animación

double DrumVisualizerAudioProcessorEditor::timeToAnimatedX(double noteTime, const juce::Rectangle<int>& area) const
{
    // Versión original para compatibility
    return timeToAnimatedX(noteTime, area, noteScrollWidth);
}

double DrumVisualizerAudioProcessorEditor::timeToAnimatedX(double noteTime, const juce::Rectangle<int>& area, double windowWidth) const
{
    if (!isPlaying)
    {
        // Vista estática: mostrar todas las notas del MIDI proporcionalmente
        double totalLength = audioProcessor.getLengthInSeconds();
        if (totalLength > 0)
        {
            double ratio = noteTime / totalLength;
            return area.getX() + (ratio * area.getWidth());
        }
        return area.getX();
    }
    else
    {
        // Vista animada: las notas se mueven de derecha a izquierda
        // La línea objetivo está a targetLineX
        // Las notas futuras aparecen desde la derecha
        
        double timeUntilNote = noteTime - currentTime;
        double scrollPixelsPerSecond = (double)area.getWidth() / windowWidth;
        
        // Posición X: línea objetivo + (tiempo hasta nota * pixels por segundo)
        return targetLineX + (timeUntilNote * scrollPixelsPerSecond);
    }
}

int DrumVisualizerAudioProcessorEditor::noteToY(int noteNumber, int lowestNote, int highestNote, const juce::Rectangle<int>& area) const
{
    int numNotes = highestNote - lowestNote + 1;
    int noteIndex = highestNote - noteNumber; // Invertir para que las notas altas estén arriba
    float noteHeightLocal = (float)area.getHeight() / (float)numNotes;  // Renombrar para evitar conflicto
    return area.getY() + (int)(noteIndex * noteHeightLocal + noteHeightLocal * 0.1f); // Pequeño offset para centrar
}

juce::String DrumVisualizerAudioProcessorEditor::getNoteNameFromNumber(int noteNumber) const
{
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int octave = (noteNumber / 12) - 1;
    int noteIndex = noteNumber % 12;
    return juce::String(noteNames[noteIndex]) + juce::String(octave);
}
