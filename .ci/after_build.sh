#!/bin/bash

case $OS in
  linux)
  {
    mv build/JirachiAdvancer .
    zip -r JirachiAdvancer-linux.zip JirachiAdvancer
    sha256sum JirachiAdvancer-linux.zip > JirachiAdvancer-linux.zip.sha256
  } ;;  
  macOS)
  {
    mv build/JirachiAdvancer .
    zip -r JirachiAdvancer-macOS.zip JirachiAdvancer
    shasum -a 256 JirachiAdvancer-macOS.zip > JirachiAdvancer-macOS.zip.sha256
  } ;;
esac