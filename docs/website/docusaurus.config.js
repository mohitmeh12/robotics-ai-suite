// @ts-check

const path = require("path");

const sphinxDocs = require("./plugins/sphinx-docs");
const configBaseUrl = (process.env.BASE_URL || "/").replace(/\/?$/, "/");

// Header text for the generated llms.txt (see plugins/llms-text).
const LLMS_SITE_DESCRIPTION =
  "The Intel\u00ae Robotics AI Suite is a full-stack platform for physical AI at the edge: a real-time Linux kernel with Intel\u00ae TCC, ROS 2 middleware, PTP-synchronized sensor pipelines, EtherCAT and CAN motion control, and OpenVINO-powered AI toolkits for vision, generative, and embodied AI on Intel\u00ae Core\u2122 Ultra SoCs.";

// Per-sidebar title/description blocks for llms.txt. Keys match sidebars.js.
const LLMS_SIDEBARS_CONFIG = {
  docsSidebar: {
    title: "Development Stack",
    description:
      "The layered bring-up stack: development kits, OS setup, real-time, middleware, sensors, models, and frameworks.",
  },
};

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: "Robotics AI Suite",
  tagline: "One x86 box that senses, thinks, and moves in real time.",
  favicon: "development-stack/_static/logo.svg",

  url: process.env.SITE_URL || "http://localhost:3000",
  baseUrl: process.env.BASE_URL || "/",
  trailingSlash: true,

  headTags: [
    {
      tagName: "script",
      attributes: {},
      innerHTML: "window.wapSinglePage = true;",
    },
    {
      tagName: "script",
      attributes: {
        src: `${configBaseUrl}common-includes/head-bootstrap.v1.js`,
        defer: true,
      },
    },
    {
      tagName: "script",
      attributes: {},
      innerHTML: `if (location.pathname.endsWith("/index.html")) {
  const canonicalUrl = new URL(location.href);
  canonicalUrl.pathname = canonicalUrl.pathname.slice(0, -"index.html".length);
  history.replaceState(history.state, "", canonicalUrl);
}`,
    },
  ],

  onBrokenLinks: "throw",
  onBrokenMarkdownLinks: "throw",

  // `static` holds site assets; the second entry is the staged Sphinx HTML
  // (see plugins/sphinx-docs.js), served verbatim at /development-stack/.
  staticDirectories: ["static", sphinxDocs.STAGING_DIR],

  // AI Models catalog is backed by a Hugging Face organization. `hfToken` is a
  // read token used ONLY to reach a private staging org during testing; it is
  // baked into the client bundle, so never set HF_TOKEN for a public/prod build.
  customFields: {
    hfOrg: process.env.HF_ORG || "OpenVINO",
    hfToken: process.env.HF_TOKEN || null,
    // Catalog membership marker. Only repos carrying this tag are listed; it is
    // ANDed into every models API query so experimental repos in the org are
    // excluded. Every catalog model must carry this tag.
    hfCatalogTag: process.env.HF_CATALOG_TAG || "",
    // Domain filter options. `label` is shown in the UI; `tag` is the Hugging
    // Face collection tag sent as the models API `filter` value.
    hfDomains: [
      { label: "Physical AI", tag: "physical-ai" },
      { label: "Vision AI", tag: "vision-ai" },
      { label: "Gen AI", tag: "gen-ai" },
    ],
    // Chipset filter options live in src/data/models/chipsets.ts alongside the
    // rest of the catalog data.
  },

  i18n: {
    defaultLocale: "en",
    locales: ["en"],
  },

  presets: [
    [
      "classic",
      /** @type {import('@docusaurus/preset-classic').Options} */
      ({
        // All docs now live in the Sphinx "Development Stack" site (served at
        // /development-stack/); no Docusaurus-rendered docs remain.
        docs: false,
        blog: false,
        sitemap: {
          ignorePatterns: ["**/models/:slug/"],
        },
        theme: {
          customCss: "./src/css/custom.css",
        },
      }),
    ],
  ],

  plugins: [
    sphinxDocs,
    require.resolve("./plugins/site-search"),
    require.resolve("./plugins/model-routes.js"),
    [
      require.resolve("./plugins/llms-text"),
      {
        siteDescription: LLMS_SITE_DESCRIPTION,
        sidebarsConfig: LLMS_SIDEBARS_CONFIG,
        // Export raw markdown for every doc page so each page's AI actions resolve.
        shouldExportFile: () => true,
      },
    ],
  ],

  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      navbar: {
        title: "Robotics Dev Resources",
        items: [
          {
            // `pathname://` links to the staged static Sphinx site directly,
            // bypassing the SPA router and broken-link checks.
            to: "pathname:///development-stack/ai-suite-robotics/",
            label: "Development Stack",
            target: "_self",
            position: "left",
          },
          {
            to: "/models/",
            label: "AI Models",
            position: "left",
          },
          {
            // `pathname://` links to the staged static Sphinx site directly,
            // bypassing the SPA router and broken-link checks.
            to: "pathname:///development-stack/hardware_blueprints/",
            label: "Blueprints",
            target: "_self",
            position: "left",
          },
          {
            href: "https://github.com/open-edge-platform/robotics-ai-suite",
            label: "GitHub",
            position: "right",
          },
          {
            type: "search",
            position: "right",
          },
        ],
      },
      footer: {
        style: "dark",
        copyright: '<div id="footer-custom-content" data-include="footer-unified"></div>',
      },
      prism: {
        theme: require("prism-react-renderer").themes.github,
        darkTheme: require("prism-react-renderer").themes.dracula,
      },
      colorMode: {
        defaultMode: "dark",
        disableSwitch: true,
        respectPrefersColorScheme: false,
      },
    }),
};

module.exports = config;
