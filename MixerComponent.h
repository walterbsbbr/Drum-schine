#ifndef MIXERCOMPONENT_H_INCLUDED
#define MIXERCOMPONENT_H_INCLUDED

#include <JuceHeader.h>
#include <array>
#include <memory>

// Forward declaration
class Mixer;

class CustomSlider : public juce::Slider
{
public:
    CustomSlider();
    
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    
private:
    juce::Image sliderFrames;
    int currentFrame = 15; // Start at middle position (frame 15 of 31)
    
    void updateFrameFromValue();
    void updateValueFromFrame();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomSlider)
};

class MixerComponent : public juce::Component,
                      public juce::Slider::Listener,
                      public juce::Button::Listener
{
public:
    MixerComponent();
    ~MixerComponent() override;
    
    void setMixer(Mixer* mixerToUse);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;
    
private:
    struct ChannelStrip
    {
        // Labels
        juce::Label channelLabel;
        
        // Controls
        std::unique_ptr<CustomSlider> volumeSlider;
        std::unique_ptr<CustomSlider> panSlider;
        juce::TextButton muteButton;
        juce::TextButton soloButton;
        
        // Value labels
        juce::Label volumeLabel;
        juce::Label panLabel;
        
        ChannelStrip(int channelIndex);
    };
    
    std::array<std::unique_ptr<ChannelStrip>, 8> channelStrips;
    
    // Master section
    std::unique_ptr<CustomSlider> masterVolumeSlider;
    juce::Label masterLabel;
    juce::Label masterVolumeLabel;
    
    Mixer* mixer = nullptr;
    
    void updateDisplayValues();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerComponent)
};

#endif // MIXERCOMPONENT_H_INCLUDED
