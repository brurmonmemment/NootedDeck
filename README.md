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

## current status

forking nootedred for our purposes, getting to navi 24 logic somewhat soon

CURRENT OBJECTIVE: finish forking NootedRed

after that, follow ChefKiss' roadmap to Steam Deck "greatness"

<hr>

### roadmap

|- [] develop navi 24-ish support via nootrx' navi 23 logic in NootRX

|- [] rewrite firmware injection method to align with the linux driver unless i can somehow figure out how to get or make the files that nootedred uses and understand how its loaded compared to linux's amdgpu

|- [X] modify the model code to only include LCD and OLED ids

|- [X] write van gogh specific code for X6000 and X6000 framebuffer

|

|- [] merge navi 21-23 logic from nootrx and add prev mentioned navi 24 code

|

|- [] merge rdna 2 (igpu) specific code & sprinkle in some steam deck bits

|

|- [not even close] finish!!!

<hr>

### credits (definitely updating as the project continues to be worked on)

- ChefKissInc for NootedRed and NootRX, without them absolutely 0% of this wouldve been possible, so huge thanks to them

