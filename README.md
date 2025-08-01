# NootedDeck
🛈 brurmonemt 2025

A kext designed to get the Steam Deck's LCD and OLED GPUs working (Van Gogh)

**[!] THIS PROJECT IS NOT FOR PROFIT AT ALL. IF YOU PAID FOR IT, PLEASE DEMAND YOUR MONEY BACK. YOU HAVE BEEN SCAMMED**

<hr>

### DISCLAIMER
- This was **NOT** made by ChefKiss or their devs, but this is a fork of their work, more specifically [NootedRed](https://github.com/ChefKissInc/NootedRed/) and [NootRX](https://github.com/ChefKissInc/NootRX) and their source code.
- The kext is more of a personal rework of the projects, and I don't claim credit for any of the base code and logic (excluding Navi 24 and some RDNA 2 code if anything new's added).

**No copyright infringement intended**

<hr>

## Current Status

Manually forking NootedRed to have a better understanding of how the code works & make changes as we go.

### Current Objective: finish forking NootedRed (need to work on AMDGPUDrivers in Kext/Headers)

<hr>

# [Roadmap]
## Stage 1A: Navi 24 (firmware) logic
- [ ] develop navi 24-ish support via nootrx' navi 23 logic in NootRX locally
- [ ] rewrite firmware injection method
    - either start from the ground up and use amdgpu's firmware blobs and code
    - OR figure out how the hell NootedRed handles this and where their files come from

## Stage 1B: Van Gogh specifics
- [X] modify the model code to only include LCD and OLED ids
- [X] write van gogh specific code for X6000 and X6000 framebuffer

## Stage 2: Integrate modern Navi logic into NootedDeck
- [ ] merge previously mentioned navi 24 code into NootedDeck and resolve dependencies
- [ ] merge RDNA 2 specific code & sprinkle in some VG-only bits

## Stage 3: Final
- [ ] start doing some real testing on the steam decks
- [ ] debug issues and do some polishing n fixing
- [ ] clean up this spaghetti code
- [14%] FINISH!

<hr>

### credits (definitely updating as the project continues to be worked on)

- ChefKissInc - NootedRed and NootRX dev, without them absolutely 0% of this wouldve been possible, so huge thanks to them
    - Consider donating to them via [ko-fi](https://ko-fi.com/chefkiss) or BTC `bc1qgu56kptepex2csuzl5nhzc4vxuj8c6ggjzhcem`
- brurmonemt - NootedDeck project leader
- tiktop101 - Doesn't know much C++ but is still learning and supporting me, so big thanks to them too 