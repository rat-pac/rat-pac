#!/bin/sh

export PHYSLIST=QGSP_BERT_EMV
rat mac/isotope.mac
mv watchman_isotope.root watchman_QGSP_BERT_EMV.root

export PHYSLIST=QGSP_BERT_EMX
rat mac/isotope.mac
mv watchman_isotope.root watchman_QGSP_BERT_EMX.root

export PHYSLIST=QGSP_BERT
rat mac/isotope.mac
mv watchman_isotope.root watchman_QGSP_BERT.root

export PHYSLIST=QGSP_BIC
rat mac/isotope.mac
mv watchman_isotope.root watchman_QGSP_BIC.root

export PHYSLIST=QBBC
rat mac/isotope.mac
mv watchman_isotope.root watchman_QBBC.root

export PHYSLIST=QBBC_EMZ
rat mac/isotope.mac
mv watchman_isotope.root watchman_QGSP_EMZ.root

export PHYSLIST=FTFP_BERT
rat mac/isotope.mac
mv watchman_isotope.root watchman_BERT.root

export PHYSLIST=QGSP_FTFP_BERT
rat mac/isotope.mac
mv watchman_isotope.root watchman_FTFP_BERT.root
