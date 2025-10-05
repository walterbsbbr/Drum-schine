#include "MixerComponent.h"
#include "Mixer.h"

// ============================================================================
// CustomSlider Implementation
// ============================================================================

CustomSlider::CustomSlider()
{
    // Load the 31-frame vertical slider PNG
    sliderFrames = juce::ImageCache::getFromMemory(BinaryData::MB_SLIDER_png, BinaryData::MB_SLIDER_pngSize);
    
    // Set slider properties
    setSliderStyle(juce::Slider::LinearVertical);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setRange(0.0, 1.0, 0.01);
    setValue(0.5); // Start at middle
    
    updateFrameFromValue();
}

void CustomSlider::paint(juce::Graphics& g)
{
    if (sliderFrames.isValid())
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Calculate frame dimensions
        int frameWidth = sliderFrames.getWidth();
        int frameHeight = sliderFrames.getHeight() / 31; // 31 frames
        
        // Calculate which frame to show based on current frame
        int frameY = currentFrame * frameHeight;
        
        // Create sub-image for the current frame
        juce::Image currentFrameImage = sliderFrames.getClippedImage(
            juce::Rectangle<int>(0, frameY, frameWidth, frameHeight));
        
        // Draw the frame scaled to fit the component
        g.drawImageWithin(currentFrameImage,
                        0, 0, getWidth(), getHeight(),
                        juce::RectanglePlacement::fillDestination);
    }
    else
    {
        // Fallback if PNG not found
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xff333333));
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Draw slider track
        g.setColour(juce::Colour(0xff666666));
        auto trackBounds = bounds.reduced(bounds.getWidth() * 0.3f, 4.0f);
        g.fillRoundedRectangle(trackBounds, 2.0f);
        
        // Draw thumb
        float thumbY = bounds.getHeight() * (1.0f - (float)getValue());
        g.setColour(juce::Colour(0xff4a9eff));
        g.fillEllipse(bounds.getX(), thumbY - 4, bounds.getWidth(), 8);
    }
}

void CustomSlider::mouseDown(const juce::MouseEvent& event)
{
    mouseDrag(event);
}

void CustomSlider::mouseDrag(const juce::MouseEvent& event)
{
    // Convert mouse Y position to frame number (inverted for vertical slider)
    float mouseY = (float)event.getPosition().getY();
    float componentHeight = (float)getHeight();
    
    // Calculate frame (0 = top = max value, 30 = bottom = min value)
    int newFrame = (int)((mouseY / componentHeight) * 30.0f);
    newFrame = juce::jlimit(0, 30, newFrame);
    
    if (newFrame != currentFrame)
    {
        currentFrame = newFrame;
        updateValueFromFrame();
        repaint();
    }
}

void CustomSlider::updateFrameFromValue()
{
    // Convert slider value (0.0-1.0) to frame number (30-0, inverted)
    currentFrame = 30 - (int)(getValue() * 30.0f);
    currentFrame = juce::jlimit(0, 30, currentFrame);
}

void CustomSlider::updateValueFromFrame()
{
    // Convert frame number (0-30) to slider value (1.0-0.0, inverted)
    double newValue = (30 - currentFrame) / 30.0;
    setValue(newValue, juce::sendNotificationSync);
}

// ============================================================================
// ChannelStrip Implementation
// ============================================================================

MixerComponent::ChannelStrip::ChannelStrip(int channelIndex)
{
    // Channel label
    channelLabel.setText(juce::String(channelIndex + 1), juce::dontSendNotification);
    channelLabel.setJustificationType(juce::Justification::centred);
    channelLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    
    // Volume slider
    volumeSlider = std::make_unique<CustomSlider>();
    volumeSlider->setValue(0.8); // Default 80%
    
    // Pan slider
    panSlider = std::make_unique<CustomSlider>();
    panSlider->setValue(0.5); // Default center
    
    // Mute button
    muteButton.setButtonText("M");
    muteButton.setClickingTogglesState(true);
    muteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff444444));
    muteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffff4444));
    
    // Solo button
    soloButton.setButtonText("S");
    soloButton.setClickingTogglesState(true);
    soloButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff444444));
    soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffffff44));
    
    // Value labels
    volumeLabel.setText("80", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centred);
    volumeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    volumeLabel.setFont(juce::FontOptions(10.0f));
    
    panLabel.setText("C", juce::dontSendNotification);
    panLabel.setJustificationType(juce::Justification::centred);
    panLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    panLabel.setFont(juce::FontOptions(10.0f));
}

// ============================================================================
// MixerComponent Implementation
// ============================================================================

MixerComponent::MixerComponent()
{
    // Create channel strips
    for (int i = 0; i < 8; ++i)
    {
        channelStrips[i] = std::make_unique<ChannelStrip>(i);
        auto& strip = *channelStrips[i];
        
        // Add components and listeners
        addAndMakeVisible(strip.channelLabel);
        addAndMakeVisible(*strip.volumeSlider);
        addAndMakeVisible(*strip.panSlider);
        addAndMakeVisible(strip.muteButton);
        addAndMakeVisible(strip.soloButton);
        addAndMakeVisible(strip.volumeLabel);
        addAndMakeVisible(strip.panLabel);
        
        strip.volumeSlider->addListener(this);
        strip.panSlider->addListener(this);
        strip.muteButton.addListener(this);
        strip.soloButton.addListener(this);
        
        // Set component IDs for identification
        strip.volumeSlider->setComponentID("vol_" + juce::String(i));
        strip.panSlider->setComponentID("pan_" + juce::String(i));
        strip.muteButton.setComponentID("mute_" + juce::String(i));
        strip.soloButton.setComponentID("solo_" + juce::String(i));
    }
    
    // Master volume
    masterVolumeSlider = std::make_unique<CustomSlider>();
    masterVolumeSlider->setValue(0.8);
    masterVolumeSlider->addListener(this);
    masterVolumeSlider->setComponentID("master_vol");
    addAndMakeVisible(*masterVolumeSlider);
    
    masterLabel.setText("MASTER", juce::dontSendNotification);
    masterLabel.setJustificationType(juce::Justification::centred);
    masterLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(masterLabel);
    
    masterVolumeLabel.setText("80", juce::dontSendNotification);
    masterVolumeLabel.setJustificationType(juce::Justification::centred);
    masterVolumeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    masterVolumeLabel.setFont(juce::FontOptions(10.0f));
    addAndMakeVisible(masterVolumeLabel);
}

MixerComponent::~MixerComponent() = default;

void MixerComponent::setMixer(Mixer* mixerToUse)
{
    mixer = mixerToUse;
    updateDisplayValues();
}

void MixerComponent::paint(juce::Graphics& g)
{
    // Dark mixer background
    g.fillAll(juce::Colour(0xff1a1a1a));
    
    // Draw channel separators
    g.setColour(juce::Colour(0xff333333));
    for (int i = 1; i < 8; ++i)
    {
        int x = i * (getWidth() / 9); // 8 channels + master
        g.drawVerticalLine(x, 0, getHeight());
    }
    
    // Draw master separator
    int masterX = 8 * (getWidth() / 9);
    g.setColour(juce::Colour(0xff555555));
    g.drawVerticalLine(masterX, 0, getHeight());
    
    // Labels
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("VOL", 10, 120, 30, 20, juce::Justification::centred);
    g.drawText("PAN", 10, 220, 30, 20, juce::Justification::centred);
}

void MixerComponent::resized()
{
    auto bounds = getLocalBounds();
    int stripWidth = bounds.getWidth() / 9; // 8 channels + master
    
    // Layout channel strips
    for (int i = 0; i < 8; ++i)
    {
        auto& strip = *channelStrips[i];
        int x = i * stripWidth;
        
        // Channel label at top
        strip.channelLabel.setBounds(x + 5, 10, stripWidth - 10, 20);
        
        // Volume slider
        strip.volumeSlider->setBounds(x + stripWidth/2 - 15, 40, 30, 80);
        strip.volumeLabel.setBounds(x + 5, 125, stripWidth - 10, 15);
        
        // Pan slider
        strip.panSlider->setBounds(x + stripWidth/2 - 15, 150, 30, 80);
        strip.panLabel.setBounds(x + 5, 235, stripWidth - 10, 15);
        
        // Buttons
        strip.muteButton.setBounds(x + 5, 260, 25, 25);
        strip.soloButton.setBounds(x + stripWidth - 30, 260, 25, 25);
    }
    
    // Master section
    int masterX = 8 * stripWidth;
    masterLabel.setBounds(masterX + 5, 10, stripWidth - 10, 20);
    masterVolumeSlider->setBounds(masterX + stripWidth/2 - 15, 40, 30, 80);
    masterVolumeLabel.setBounds(masterX + 5, 125, stripWidth - 10, 15);
}

void MixerComponent::sliderValueChanged(juce::Slider* slider)
{
    if (mixer == nullptr) return;
    
    auto componentID = slider->getComponentID();
    
    if (componentID.startsWith("vol_"))
    {
        int channel = componentID.substring(4).getIntValue();
        mixer->setChannelVolume(channel, (float)slider->getValue());
        
        // Update volume label
        int volumePercent = (int)(slider->getValue() * 100);
        channelStrips[channel]->volumeLabel.setText(juce::String(volumePercent), juce::dontSendNotification);
    }
    else if (componentID.startsWith("pan_"))
    {
        int channel = componentID.substring(4).getIntValue();
        float panValue = ((float)slider->getValue() - 0.5f) * 2.0f; // Convert 0-1 to -1 to 1
        mixer->setChannelPan(channel, panValue);
        
        // Update pan label
        if (panValue < -0.1f)
            channelStrips[channel]->panLabel.setText("L" + juce::String((int)(panValue * -50)), juce::dontSendNotification);
        else if (panValue > 0.1f)
            channelStrips[channel]->panLabel.setText("R" + juce::String((int)(panValue * 50)), juce::dontSendNotification);
        else
            channelStrips[channel]->panLabel.setText("C", juce::dontSendNotification);
    }
    else if (componentID == "master_vol")
    {
        mixer->setMasterVolume((float)slider->getValue());
        
        // Update master volume label
        int volumePercent = (int)(slider->getValue() * 100);
        masterVolumeLabel.setText(juce::String(volumePercent), juce::dontSendNotification);
    }
}

void MixerComponent::buttonClicked(juce::Button* button)
{
    if (mixer == nullptr) return;
    
    auto componentID = button->getComponentID();
    
    if (componentID.startsWith("mute_"))
    {
        int channel = componentID.substring(5).getIntValue();
        mixer->setChannelMute(channel, button->getToggleState());
    }
    else if (componentID.startsWith("solo_"))
    {
        int channel = componentID.substring(5).getIntValue();
        mixer->setChannelSolo(channel, button->getToggleState());
    }
}

void MixerComponent::updateDisplayValues()
{
    if (mixer == nullptr) return;
    
    // Update all channel controls to match mixer state
    for (int i = 0; i < 8; ++i)
    {
        auto& strip = *channelStrips[i];
        
        strip.volumeSlider->setValue(mixer->getChannelVolume(i), juce::dontSendNotification);
        strip.panSlider->setValue((mixer->getChannelPan(i) + 1.0f) * 0.5f, juce::dontSendNotification); // Convert -1 to 1 to 0-1
        strip.muteButton.setToggleState(mixer->isChannelMuted(i), juce::dontSendNotification);
        strip.soloButton.setToggleState(mixer->isChannelSoloed(i), juce::dontSendNotification);
    }
    
    masterVolumeSlider->setValue(mixer->getMasterVolume(), juce::dontSendNotification);
}
