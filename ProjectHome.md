## LNK Parser ##
LNK Parser allows you to view the content of Windows shortcut (.LNK) files.


---


### Command-line Usage ###
You can run the program from a command-line and supply a shortcut file or a directory. If a directory is supplied, the directory and any sub-directories will be scanned for shortcut files.

Example 1: lnk\_parser\_cmd.exe shortcut.lnk

Example 2: lnk\_parser\_cmd.exe C:\Users\User\Desktop

There are additional options that can be specified to allow you to generate various reports of the shortcut file information.

The following is a list of supported options:

  * -o Sets the output directory for reports.
  * -w Generates an HTML report.
  * -c Generates a comma-separated values (CSV) report.
  * -r Remnant and unprocessed data will be dumped to the HTML report.
  * -s Prevents the console from outputting information.

Example: lnk\_parser\_cmd.exe -o Output -w -c -r -s shortcut.lnk

If you wish to save the command-line output to a file, you can use the redirect operator.

Example: lnk\_parser\_cmd.exe shortcut.lnk > output.txt

You can speed up the extraction and report generation by redirecting the command-line output to the NUL device, or by using the -s option mentioned above.

Example 1: lnk\_parser\_cmd.exe shortcut.lnk > NUL

Example 2: lnk\_parser\_cmd.exe -s shortcut.lnk

The program can also be run normally (not through the command-line). It will ask for the shortcut file or directory, and whether you wish to include a report(s).

When specifying a file or directory path, it's best to surround it with quotes since spaces within the path might be interpreted as separate options.


---


### Notes and References ###

A good starting point to understanding the shortcut file format can be found [here](http://msdn.microsoft.com/en-us/library/dd871305(v=prot.13).aspx). There are two important structures that are found within the format, but they're not given much detail in relation to it. The first is the [Item ID List](http://msdn.microsoft.com/en-us/library/windows/desktop/bb773321(v=vs.85).aspx), and the second is the [Property Store](http://msdn.microsoft.com/en-us/library/dd871346(v=prot.13).aspx). The content in the Item ID List is not documented (probably for security reasons), but it's primarily used to resolve the path that the shortcut links to. This structure can contain property stores and vice versa. I've managed to parse the most common items, and everything that's unknown/unsupported is noted in the output.

The Property Store structure holds the meat of any additional information. It stores metadata from the target file and allows, for example, folder columns to display details such as album title, image width/height, video length, etc. Property stores can have data streams which commonly contain ID lists, timestamps, and language codes. There is one particular stream that relates to Windows search results. It's approximately 10,000 bytes in length and contains what I believe to be remnant memory from a shared system location. I'm 99.9% certain of this as I've seen truncated images, ID lists, and various other data that doesn't offer a definable structure within the stream. It can be parsed up to a point, but everything beyond that (I assume) can't be determined.

If you're in the forensics field and are interested in knowing how to use some of the information within a shortcut file, you can find an excellent article written by Harry Parsonage [here](http://computerforensics.parsonage.co.uk/downloads/TheMeaningofLIFE.pdf).