#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFPageObjectHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QUtil.hh>

// This program demonstrates use of form XObjects to overlay a page
// from one file onto all pages of another file. The qpdf program's
// --overlay and --underlay options provide a more general version of
// this capability.

static char const* whoami = 0;

void usage()
{
    std::cerr << "Usage: " << whoami << " infile pagefile outfile"
	      << std::endl
	      << "Stamp page 1 of pagefile on every page of infile,"
              << " writing to outfile"
	      << std::endl;
    exit(2);
}

static void stamp_page(char const* infile,
                       char const* stampfile,
                       char const* outfile)
{
    QPDF inpdf;
    inpdf.processFile(infile);
    QPDF stamppdf;
    stamppdf.processFile(stampfile);

    // Get first page from other file
    QPDFPageObjectHelper stamp_page_1 =
        QPDFPageDocumentHelper(stamppdf).getAllPages().at(0);
    // Convert page to a form XObject
    QPDFObjectHandle foreign_fo = stamp_page_1.getFormXObjectForPage();
    // Copy form XObject to the input file
    QPDFObjectHandle stamp_fo = inpdf.copyForeignObject(foreign_fo);

    // For each page...
    std::vector<QPDFPageObjectHelper> pages =
        QPDFPageDocumentHelper(inpdf).getAllPages();
    for (std::vector<QPDFPageObjectHelper>::iterator iter = pages.begin();
         iter != pages.end(); ++iter)
    {
        QPDFPageObjectHelper& ph = *iter;

        // Find a unique resource name for the new form XObject
        QPDFObjectHandle resources = ph.getAttribute("/Resources", true);
        int min_suffix = 1;
        std::string name = resources.getUniqueResourceName("/Fx", min_suffix);

        // Generate content to place the form XObject centered within
        // destination page's trim box.
        std::string content =
            ph.placeFormXObject(
                stamp_fo, name, ph.getTrimBox().getArrayAsRectangle());
        if (! content.empty())
        {
            // Append the content to the page's content. Surround the
            // original content with q...Q to the new content from the
            // page's original content.
            resources.mergeResources(
                QPDFObjectHandle::parse("<< /XObject << >> >>"));
            resources.getKey("/XObject").replaceKey(name, stamp_fo);
            ph.addPageContents(
                QPDFObjectHandle::newStream(&inpdf, "q\n"), true);
            ph.addPageContents(
                QPDFObjectHandle::newStream(&inpdf, "\nQ\n" + content), false);
        }
    }

    QPDFWriter w(inpdf, outfile);
    w.setStaticID(true);        // for testing only
    w.write();
}

int main(int argc, char* argv[])
{
    whoami = QUtil::getWhoami(argv[0]);

    // For libtool's sake....
    if (strncmp(whoami, "lt-", 3) == 0)
    {
	whoami += 3;
    }

    if (argc != 4)
    {
        usage();
    }
    char const* infile = argv[1];
    char const* stampfile = argv[2];
    char const* outfile = argv[3];

    try
    {
        stamp_page(infile, stampfile, outfile);
    }
    catch (std::exception &e)
    {
	std::cerr << whoami << ": " << e.what() << std::endl;
	exit(2);
    }
    return 0;
}
