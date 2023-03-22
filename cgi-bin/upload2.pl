#!/usr/bin/perl -w

use CGI;
$upload_dir = "/images";
$query = new CGI;
$filename = $query->param("filename");
$upload_filehandle = $query->upload("filename");
open UPLOADFILE, ">$upload_dir/$filename";
while ( <$upload_filehandle> )
{
	print UPLOADFILE;
}

close UPLOADFILE;

print $query->header ( );
print <<END_HTML;
<HTML>
<HEAD>
<TITLE>Thanks!</TITLE>
</HEAD>

<BODY>

<P>Thanks for uploading your photo!</P>
<P>Your photo:</P>
<img src="/images/$filename" border="0">

</BODY>
</HTML>

END_HTML
