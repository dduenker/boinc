<?php
require_once("docutil.php");

page_head("Cross-project identification");

echo "
BOINC supports and encourages 'third-party leaderboards',
i.e. web sites that show statistics from BOINC projects.
Such web sites can obtain raw data via XML downloads.
<p>
Leaderboard sites may show statistics from several BOINC projects,
and some people may want to see their credit
summed across all the projects in which they participate.
This turns out to be a little tricky.
When should accounts on different projects be considered equivalent?
The simplest answer is: when they have the same email address.
But we can't export email addresses.
And we can't export hashed email addresses,
because spammers could enumerate feasible email addresses
and compare them with the hashed addresses.

<p>
BOINC uses the following system:
<ul>
<li>
Each account is assigned a 'cross-project identifier' (CPID)
when it's created; it's a long random string.
<li>
When a scheduling server replies to an RPC,
it includes the account's CPID and hashed email address.
<li>
The BOINC client stores the CPID and hashed email address
of each account to which it's attached.
<li>
When the BOINC client makes an RPC request to a scheduling server,
it includes the greatest (in terms of string comparison) CPID
from among projects with the same hashed email.
<li>
If the scheduling server receives a CPID different
from the one in its database, it updates the database with the new CPID.
<li>
User elements in the XML download files include
a hash of (email address, CPID);
this 'export' CPID serves as a unique identifier of all
accounts with that email address.
(The last step, hashing with the email address,
prevents people from impersonating other people).
</ul>

This system provides cross-project identification based on email address,
without publicizing any information from which
email addresses could be derived.
<p>
This process is illustrated below:
<br>
<img src=CPI_large.jpg>

";

page_tail();
?>
