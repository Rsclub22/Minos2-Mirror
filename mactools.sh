#!/bin/bash


brew update -y
brew upgrade -y

brew install git 
# brew install git-gui -y
brew install wget
brew install cmake autoconf

brew install libasound2 -y
brew install libasound2-dev -y

brew autoremove -y

