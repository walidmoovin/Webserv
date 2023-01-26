# Webserv

[Subject](webserv.pdf)

## ToDo/ToFix:

- [] organize answer structure
- [] default html pages (404, 503 ) ??
- [] limit client body
- [] DELETE method
- [] POST method correctly
  - [] hard test segmented request
  - [] verify upload of files
  - [] what to do with content ?
- [] STRESS TEST!! Server must stay available!!
- [] Server shout never indefinitly wait.
- [] PUT non-text file may finish because of eof char.

## Notes:
- too low keep_alive amount badly close sockets and produce infinit client load.

## Behavior:

### Configuration Parsing:
[![](https://mermaid.ink/img/pako:eNqFlG1vmzAQx78K8qu2IghSQotVpWq7N5vadEq2F9uYKgOXhBXsyJhpaZTvvsMOgXQki4SC7_-7Bx133pBEpEAoKRVT8CFjC8mKwe9hxH9c_LQGg7H1afY8-cxkCfLsnFqJ4POXeZYDZwVEXHsdINYm4hb-GndnVQtn58Zq-MbWsF1eLbPSUeIVePYGkrapGvAQMD7NyVmA-lIfmnxtzj6mm39XQx0O_1r79p9A6xVYNzfJUmQJjMedwvoy6PLQg36cONrztuNQn2tAt-M5_gWJotbD1-njt6PUY1Yiczed3vUyUuR5zJJXisXK0wCP_wPEQuQdohHaWmZKZnxxiphURQzyFHGPSYDxFun0oqXSd3rdheOqqeu4bqo6ru9qqoH3uhkBCaqSvHcIjIPppKaotWTlk5CgJ6LbTxNkP_Pt0NA1lL0cDiblwki7udyvkvl2Qr1McJ-Ntj82zmiLODoe7Ot-4olNCpAFy1K8DvRmREQtAXePUHxNYc6qXEUEQyDKKiVma54QqmQFNqlWaXuBEDpneYlWSDMl5JO5YvRNY5MV44RuyB9Ch77vjNxr79K_Gl27V0EQ2GRN6OAycHw3GLqhF7pe4AejrU3ehMCwnuMF4cgPQ3-ISoCPjvddi3Uh27-CXXGd?type=png)](https://mermaid.live/edit#pako:eNqFlG1vmzAQx78K8qu2IghSQotVpWq7N5vadEq2F9uYKgOXhBXsyJhpaZTvvsMOgXQki4SC7_-7Bx133pBEpEAoKRVT8CFjC8mKwe9hxH9c_LQGg7H1afY8-cxkCfLsnFqJ4POXeZYDZwVEXHsdINYm4hb-GndnVQtn58Zq-MbWsF1eLbPSUeIVePYGkrapGvAQMD7NyVmA-lIfmnxtzj6mm39XQx0O_1r79p9A6xVYNzfJUmQJjMedwvoy6PLQg36cONrztuNQn2tAt-M5_gWJotbD1-njt6PUY1Yiczed3vUyUuR5zJJXisXK0wCP_wPEQuQdohHaWmZKZnxxiphURQzyFHGPSYDxFun0oqXSd3rdheOqqeu4bqo6ru9qqoH3uhkBCaqSvHcIjIPppKaotWTlk5CgJ6LbTxNkP_Pt0NA1lL0cDiblwki7udyvkvl2Qr1McJ-Ntj82zmiLODoe7Ot-4olNCpAFy1K8DvRmREQtAXePUHxNYc6qXEUEQyDKKiVma54QqmQFNqlWaXuBEDpneYlWSDMl5JO5YvRNY5MV44RuyB9Ch77vjNxr79K_Gl27V0EQ2GRN6OAycHw3GLqhF7pe4AejrU3ehMCwnuMF4cgPQ3-ISoCPjvddi3Uh27-CXXGd)

### Server fonctionment:
- set timeout.

