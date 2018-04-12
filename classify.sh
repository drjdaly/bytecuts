RuleList="acl1_seed_1"
SourceDir=./
PacketDir=./
OutputDir=../Output/

Program=./main
Validate=./validate

echo $RuleList #
if [ ! -d ${OutputDir}/Results ]; then
	mkdir -p ${OutputDir}/Results/
fi

File=${SourceDir}/${RuleList}.rules
Output=${OutputDir}/${RuleList}.rules.csv
Packets=${PacketDir}/${RuleList}.rules.trace
BC=${OutputDir}/Results/BC_${RuleList}.txt
#SS=${OutputDir}/Results/SS_${RuleList}.txt

${Program} Rules=${File} Packets=${Packets} Stats=${Output} BC.TurningPoint=0.01 BC.BadFraction=0.02 Results=${BC} 

#${Validate} Rules=${File} Packets=${Packets} ByteCuts=${BC} SmartSplit=${SS}

