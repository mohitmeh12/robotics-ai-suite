// Intel silicon shown on docs/hardware/index.md. Two static entries:
// the on-robot SoC (Intel® Core™ Ultra Series 3 Processors) and the discrete inference GPU
// (Intel® Data Center GPU Flex Series). Facts only, no fabricated figures.

const silicon = [
  {
    id: 'panther-lake',
    name: 'Intel® Core™ Ultra Series 3 Processors',
    tier: 'On-robot compute',
    description:
      'A single SoC that combines CPU, Intel® Arc™ graphics, and Intel NPU, so perception, control, and AI inference run on the same chip in a power envelope that fits on a robot.',
    points: [
      'CPU, Intel® Arc™ graphics (12 Xe cores), and Intel NPU 5.0, delivering up to 180 TOPS of combined AI compute',
      'Heterogeneous engines let you place each workload on the best-suited unit',
      'No discrete accelerator required for on-robot perception and control',
    ],
  },
  {
    id: 'crescent-island',
    name: 'Intel® Data Center GPU Flex Series',
    tier: 'On-premise inference',
    description:
      'An air-cooled data center GPU built on Intel graphics architecture, optimized for tokens per watt when inference workloads exceed the on-robot SoC.',
    points: [
      'Inference-optimized for performance per watt and per TCO',
      'Widest range of AI datatypes across matrix and vector, down to INT4/MxFP4',
      'Fits air-cooled solutions on a reliable open software stack',
    ],
  },
];

export default silicon;
