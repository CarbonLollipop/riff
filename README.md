# riff 🎶
A speedy CLI music player

https://github.com/CarbonLollipop/riff/assets/93494083/8af0170d-8498-41ac-9c0c-00937e5b0f7e

## Compile It
### GNU/Linux
Install headers\
`apt install libsdl2-mixer-2.0-0 libsdl2-mixer-dev libmpg123-dev libsndfile1-dev libncurses-dev g++`\
or\
`pacman -S sdl2_mixer mpg123 libsndfile`

Clone the repository, compile, and run riff
```
git clone https://github.com/CarbonLollipop/riff.git
cd riff
make
make install # optional, copies riff to /usr/local/bin/
./riff <song/directory of songs>
```
### Windows
Good luck

## Current Features
- Pausing
- Skipping
- Playlists
- Volume Controls
- Time Elapsed and Duration
- Progress Bar
- Seeking

## Planned Features
- Reading Audio Metadata
- Lyrics (???)

