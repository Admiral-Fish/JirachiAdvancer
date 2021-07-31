#!/bin/bash

case $OS in
  linux)
  {
    mv build/JirachiAdvancer .
    zip -r JirachiAdvancer-linux.zip PokJirachiAdvancerFinder
    sha256sum JirachiAdvancer-linux.zip > JirachiAdvancer-linux.zip.sha256
  } ;;  
  macOS)
  {
    mv build/PokeFinder.app .
    zip -r JirachiAdvancer-macOS.zip JirachiAdvancer.app
    shasum -a 256 JirachiAdvancer-macOS.zip > JirachiAdvancer-macOS.zip.sha256
  } ;;
esac