# mwx2root
MWX to ROOT file converter

MWX is a format that seems to be used by some radiosondes containing sounding data, e.g. at the South Pole. It seems to just be zip of various XML files. This tools attempts to convert those files to ROOT format. 

## Usage
` mwx2root file.mwx [-o output=file.root] [XMLFilePrefix = GpsResults] [AnotherXMLFilePrefix] [...] `


## Notes 

You can use `unzip -l` to find the XML files within the archive. This program works by calling `unzip` to extract a file into memory so if you have an insanely large XML file it will use some memory. You must have a version of unzip that supports that, which you probably do? 

