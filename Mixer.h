#ifndef MIXER_H_INCLUDED
#define MIXER_H_INCLUDED

#include <JuceHeader.h>
#include <array>

class Mixer
{
public:
    Mixer();
    ~Mixer();
    
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processChannelBuffer(int channelIndex, juce::AudioBuffer<float>& buffer, int numSamples);
    void releaseResources();
    
    // Channel controls
    void setChannelVolume(int channel, float volume);     // 0.0 to 1.0
    void setChannelPan(int channel, float pan);           // -1.0 to 1.0
    void setChannelMute(int channel, bool muted);
    void setChannelSolo(int channel, bool soloed);
    
    // Getters
    float getChannelVolume(int channel) const;
    float getChannelPan(int channel) const;
    bool isChannelMuted(int channel) const;
    bool isChannelSoloed(int channel) const;
    bool hasAnySoloedChannels() const;
    
    // Master controls
    void setMasterVolume(float volume);
    float getMasterVolume() const { return masterVolume; }
    
private:
    struct ChannelStrip
    {
        float volume = 0.8f;        // Default 80%
        float pan = 0.0f;           // Center
        bool muted = false;
        bool soloed = false;
        
        // Pan law calculation
        float leftGain = 1.0f;
        float rightGain = 1.0f;
        
        void updatePanGains();
    };
    
    std::array<ChannelStrip, 8> channels;
    float masterVolume = 0.8f;
    bool anySoloed = false;
    
    void updateSoloState();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Mixer)
};

#endif // MIXER_H_INCLUDED
