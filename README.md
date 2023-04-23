# wavy

Wavy is a command-line tool that renders VCD files as waveforms in a terminal. 
With wavy, you can quickly and easily visualize the signals in your VCD files and navigate through them with ease.

## Installation

To install wavy, simply clone the repository and build the tool

```
git clone https://github.com/domso/wavy.git
mkdir build
cd build
cmake ..
make -j
```

## Usage

To use wavy, simply run the `wavy` command followed by the name of the VCD file you want to view:

```
wavy trace.vcd
```

This will display a selection of the available signals in the VCD file. You can use the `jk` keys to navigate through the list. 
It is also possible to group and ungroup hierarchical structures by pressing the spacebar. 
Grouping is done by common signal name prefix and not by the VCD structure.

To add or remove signals from the waveform, use the `a` and `d` keys respectively.
Note that if applied to a grouped signal, all signals that belong to that group will also be added.

To view the waveform, press the tabulator key.
Once in the waveform view, you can scroll using the `jhkl` keys and use the `+` and `-` keys to zoom in and out.
