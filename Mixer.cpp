#include "Mixer.h"

Mixer::Mixer()
{
    // Initialize all channels with default pan gains
    for (auto& channel : channels)
    {
        channel.updatePanGains();
    }
}

Mixer::~Mixer() = default;

void Mixer::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Nothing special needed for basic volume/pan
}

void Mixer::processChannelBuffer(int channelIndex, juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (channelIndex < 0 || channelIndex >= 8)
        return;
    
    auto& channel = channels[channelIndex];
    
    // Check if channel should be audible
    bool shouldPlay = !channel.muted && (!anySoloed || channel.soloed);
    
    if (!shouldPlay)
    {
        buffer.clear();
        return;
    }
    
    // Apply volume and pan
    float finalVolume = channel.volume * masterVolume;
    
    if (buffer.getNumChannels() >= 2)
    {
        // Stereo processing
        auto* leftChannel = buffer.getWritePointer(0);
        auto* rightChannel = buffer.getWritePointer(1);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float monoSample = (leftChannel[sample] + rightChannel[sample]) * 0.5f;
            
            leftChannel[sample] = monoSample * finalVolume * channel.leftGain;
            rightChannel[sample] = monoSample * finalVolume * channel.rightGain;
        }
    }
    else if (buffer.getNumChannels() == 1)
    {
        // Mono processing
        auto* monoChannel = buffer.getWritePointer(0);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            monoChannel[sample] *= finalVolume;
        }
    }
}

void Mixer::releaseResources()
{
    // Nothing to release for basic mixer
}

void Mixer::setChannelVolume(int channel, float volume)
{
    if (channel >= 0 && channel < 8)
    {
        channels[channel].volume = juce::jlimit(0.0f, 1.0f, volume);
    }
}

void Mixer::setChannelPan(int channel, float pan)
{
    if (channel >= 0 && channel < 8)
    {
        channels[channel].pan = juce::jlimit(-1.0f, 1.0f, pan);
        channels[channel].updatePanGains();
    }
}

void Mixer::setChannelMute(int channel, bool muted)
{
    if (channel >= 0 && channel < 8)
    {
        channels[channel].muted = muted;
    }
}

void Mixer::setChannelSolo(int channel, bool soloed)
{
    if (channel >= 0 && channel < 8)
    {
        channels[channel].soloed = soloed;
        updateSoloState();
    }
}

float Mixer::getChannelVolume(int channel) const
{
    if (channel >= 0 && channel < 8)
        return channels[channel].volume;
    return 0.0f;
}

float Mixer::getChannelPan(int channel) const
{
    if (channel >= 0 && channel < 8)
        return channels[channel].pan;
    return 0.0f;
}

bool Mixer::isChannelMuted(int channel) const
{
    if (channel >= 0 && channel < 8)
        return channels[channel].muted;
    return false;
}

bool Mixer::isChannelSoloed(int channel) const
{
    if (channel >= 0 && channel < 8)
        return channels[channel].soloed;
    return false;
}

bool Mixer::hasAnySoloedChannels() const
{
    return anySoloed;
}

void Mixer::setMasterVolume(float volume)
{
    masterVolume = juce::jlimit(0.0f, 1.0f, volume);
}

void Mixer::updateSoloState()
{
    anySoloed = false;
    for (const auto& channel : channels)
    {
        if (channel.soloed)
        {
            anySoloed = true;
            break;
        }
    }
}

void Mixer::ChannelStrip::updatePanGains()
{
    // Equal power pan law
    float panRadians = pan * juce::MathConstants<float>::halfPi * 0.5f;
    
    leftGain = std::cos(panRadians + juce::MathConstants<float>::halfPi * 0.5f);
    rightGain = std::sin(panRadians + juce::MathConstants<float>::halfPi * 0.5f);
}
