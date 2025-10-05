#include "AppLookAndFeel.h"

AppLookAndFeel::AppLookAndFeel()
{
    // Slightly larger default text for buttons
    setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    setColour(juce::TextButton::textColourOnId,  juce::Colours::white);
}

juce::Font AppLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    // Return a Font (constructed from FontOptions) with sensible size/weight
    return withDefaultMetrics(juce::FontOptions(juce::jmin(16, buttonHeight - 8), juce::Font::bold));
}

void AppLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                          juce::Button& button,
                                          const juce::Colour& /*backgroundColour*/,
                                          bool isMouseOverButton,
                                          bool isButtonDown)
{
    auto r = button.getLocalBounds().toFloat();
    const float corner = juce::jlimit(4.0f, 12.0f, juce::jmin(r.getWidth(), r.getHeight()) * 0.25f);

    // Use the button's configured colour (handles toggle/on states)
    juce::Colour base = button.getToggleState()
                        ? button.findColour(juce::TextButton::buttonOnColourId, true)
                        : button.findColour(juce::TextButton::buttonColourId, true);

    if (isButtonDown)           base = base.darker(0.20f);
    else if (isMouseOverButton) base = base.brighter(0.10f);

    // Fill gradient for beveled look
    juce::ColourGradient grad(base.brighter(0.15f), r.getCentreX(), r.getY(),
                              base.darker(0.22f),   r.getCentreX(), r.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(r, corner);

    // Outer dark stroke
    g.setColour(juce::Colours::black.withAlpha(0.55f));
    g.drawRoundedRectangle(r, corner, 2.0f);

    // Inner light stroke
    auto inner = r.reduced(2.0f);
    g.setColour(juce::Colours::white.withAlpha(0.12f));
    g.drawRoundedRectangle(inner, corner - 2.0f, 1.2f);
}

void AppLookAndFeel::drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height)
{
    auto r = juce::Rectangle<int>(0, 0, width, height).toFloat();
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle(r, 6.0f);

    g.setColour(juce::Colours::white.withAlpha(0.18f));
    g.drawRoundedRectangle(r, 6.0f, 3.0f);

    g.setColour(juce::Colours::white);
    g.setFont(withDefaultMetrics(juce::FontOptions(12.0f)));
    g.drawFittedText(text, juce::Rectangle<int>(0, 0, width, height).reduced(6, 4),
                     juce::Justification::centred, 2);
}

juce::Rectangle<int> AppLookAndFeel::getTooltipBounds(const juce::String& tipText,
                                                      juce::Point<int> screenPos,
                                                      juce::Rectangle<int> parentArea)
{
    // Measure with GlyphArrangement (Font::getStringWidthFloat is deprecated)
    const juce::Font font = withDefaultMetrics(juce::FontOptions(12.0f));
    const int textWidth = juce::GlyphArrangement::getStringWidthInt(font, tipText);
    const int w = juce::jlimit(80, 400, textWidth + 14);
    const int h = 24 + (tipText.contains("\n") ? 14 : 0);

    auto r = juce::Rectangle<int>(screenPos.x + 12, screenPos.y + 16, w, h);
    return r.constrainedWithin(parentArea);
}
