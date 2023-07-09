package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

require "Pktgen"
-- A list of the test script for Pktgen and Lua.
-- Each command somewhat mirrors the pktgen command line versions.
-- A couple of the arguments have be changed to be more like the others.
--

prints("** pktgen.linkState('all')", pktgen.linkState("all"));
prints("** pktgen.isSending('all')", pktgen.isSending("all"));

prints("** pktgen.portSizes('all')", pktgen.portSizes("all"));
prints("** pktgen.pktStats('all')", pktgen.pktStats("all"));

prints("** pktgen.portRates('all', 'rate')", pktgen.portStats("all", "rate"));
prints("** pktgen.portStats('all', 'port')", pktgen.portStats('all', 'port'));

prints("** pktgen.portRates('0', 'rate')", pktgen.portStats("0", "rate"));
prints("** pktgen.portStats('1-2', 'port')", pktgen.portStats('1-2', 'port'));
