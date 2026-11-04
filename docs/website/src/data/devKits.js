import { INTERFACE, PHY, IPU } from './constants';

// Single source of truth for development kits. Drives both the camera
// compatibility filter (src/components/CameraTable) and the development-kit
// cards on docs/hardware/index.md.
//
// To add a kit, copy the entry and fill in:
//   name        Display name.
//   platform    Human-readable SoC/platform (shown next to the kit).
//   ipu         The kit's IPU generation (from IPU). A camera is only
//               supported if its ipuSupport includes this value.
//   interfaces  Camera interfaces the kit physically exposes (from INTERFACE).
//   mipiPhy     PHY type of the kit's MIPI connectors (from PHY). A DPHY
//               sensor on a CPHY kit is flagged "adapter required".
//   docLink     Link to the kit's landing/quick-start page.
//   connectLink Link to the kit's "Connect a camera" wiring page (drives the
//               per-camera "Compatible development kits" callout).
//   image       require()'d card image (use the @site alias for the path).
//   description Card blurb.
//   specs       Card spec bullets: { ai, memory, vision, control }.
//
// See src/data/README.md for the full maintenance guide.

const devKits = [
  {
    id: 'robinson-bay',
    name: 'Robinson Bay',
    platform: 'Intel® Core™ Ultra Series 3 Processors (Core™ Ultra X7 358H)',
    ipu: IPU.IPU75XA,
    interfaces: [INTERFACE.GMSL, INTERFACE.MIPI],
    mipiPhy: PHY.CPHY,
    docLink: '/docs/hardware/development-kits/robinson-bay/quick-start',
    connectLink: '/docs/hardware/development-kits/robinson-bay/interfaces',
    image: require('@site/../docs/hardware/img/aaeon-cexd-intrbl.png')
      .default,
    description:
      'Modular form factor supporting a wide range of robotics with integrated GMSL camera connectivity, EtherCAT for real-time controls, and additional capabilities to address diverse robotic requirements.',
    specs: {
      ai: 'CPU, Intel® Arc™ graphics (12 Xe Cores), and Intel NPU 5.0 (up to 180 TOPS)',
      memory: '64GB LPDDR5 (up to 8533MT/s)',
      vision: '8x GMSL camera interfaces, 4x USB Type-C',
      control: '4x 2.5GbE LAN (IEEE 1588 PTP), CANBus, 40-pin GPIO HAT',
    },
  },
];

export default devKits;
