Installation
------------

Save YFQuote.xla to C:\Documents and Settings\<user name>\Application Data\Microsoft\AddIns
Launch Excel
Click Tools->Addins 
Check the YF Quote addin and click OK

You can verify that the addin is loaded by via the =YF_Version() function in Excel
This should return the version number and not #NAME


Functions
---------

=YF_Version() - returns the version
=YF_Price("TICKER", ["BID"/"ASK"/"LAST"/"CLOSE"]) - returns the specified price of a stock (N.B. if no price side (bid,ask....) is specified the last price is returned)


Examples
--------

=YF_Price("VOD.L") - returns the last price for Vodafone
=YF_Price("VOD.L", "BID") - returns the bid price for Vodafone