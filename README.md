# VST3 Plugin Project Summary

## Introduction

SimpleQ is the first VST3 plugin that I've developed using the JUCE framework. It's a basic plugin that includes High and Low-Cut filters, an adjustable parametric EQ band and a real-time frequency spectrum analyzer.

## Features

### 1. Two-Band Cut Filters

#### a. Low-Cut Filter

- The low-cut filter eliminates low-frequency content from the audio signal.
- Users can choose from four different slope curves.
- A slider is provided to select the start frequency of the slope.

#### b. High-Cut Filter

- The high-cut filter removes high-frequency components from the audio.
- Similar to the low-cut filter, users can choose from four different slope curves and adjust the start frequency with a slider.

### 2. Peak Equalizer

In addition to the cut filters, the plugin includes a peak equalizer control. This component offers control over the entire range of audible frequencies and provides users with the tools to enhance or attenuate specific frequency bands in the audio:

- **Frequency**: Users can set the center frequency for the peak equalizer.
- **Gain**: Adjust the gain to boost or cut the selected frequency.
- **Q (Quality Factor)**: Control the bandwidth of the EQ curve for precise adjustments.

### 3. Frequency Spectrum Analyzer

The plugin features a built-in frequency spectrum analyzer, allowing users to visualize the frequency distribution of the audio signal in real-time.

## User Interface

The plugin provides a very basic and user-friendly interface. Users can easily adjust parameters and monitor the changes in real-time.

<p align="center">
  <img src="https://github.com/LucasVerdelho/SimpleQ/blob/main/images/SimpleQ_in_use.png" />
</p>

## TODO

- [ ] Clean up the design of the slope selection buttons.
- [ ] Add a Logo
- [ ] Add preset functionality
- [ ] Add a global bypass button
- [ ] Add more EQ bands
- [ ] Improve the design of the frequency spectrum analyzer
