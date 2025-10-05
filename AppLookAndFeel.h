#ifndef APPLOOKANDFEEL_H_INCLUDED
#define APPLOOKANDFEEL_H_INCLUDED

#include <JuceHeader.h>

class AppLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AppLookAndFeel();
    ~AppLookAndFeel() override = default;

    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool isMouseOverButton,
                              bool isButtonDown) override;

    void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override;
    juce::Rectangle<int> getTooltipBounds(const juce::String& tipText,
                                          juce::Point<int> screenPos,
                                          juce::Rectangle<int> parentArea) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
};

#endif // APPLOOKANDFEEL_H_INCLUDED
