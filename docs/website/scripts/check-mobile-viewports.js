// SPDX-FileCopyrightText: (C) 2026 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

const { chromium, firefox, webkit } = require("playwright");
const { once } = require("node:events");

const BASE_URL = process.argv[2] || process.env.TEST_URL;

const VIEWPORTS = [
  { name: "Mobile Small (iPhone SE / Galaxy Fold)", width: 320, height: 568 },
  { name: "Mobile Standard (iPhone 12/13/14/15/16)", width: 390, height: 844 },
  { name: "Mobile Large (Pixel 7 / Galaxy S21)", width: 412, height: 915 },
  { name: "Mobile Pro Max (iPhone Plus / Max)", width: 430, height: 932 },
  { name: "Small Tablet / Landscape Mobile", width: 640, height: 800 },
  { name: "Tablet Portrait (iPad Mini / Air)", width: 768, height: 1024 },
  { name: "Desktop", width: 1280, height: 800 },
];

const ROUTES = [
  { path: "/", hydrated: true },
  { path: "/models/", hydrated: true },
  { path: "/skills/", hydrated: true },
  { path: "/development-stack/ai-suite-robotics/", hydrated: false },
];

const BROWSERS = [
  { name: "Chromium", type: chromium, launchOptions: { args: ["--no-sandbox", "--disable-gpu", "--disable-dev-shm-usage"] } },
  { name: "Firefox", type: firefox },
  { name: "WebKit", type: webkit },
];

async function run() {
  const previousPort = process.env.PORT;
  if (!BASE_URL) process.env.PORT = "0";
  const server = BASE_URL ? null : require("./serve");
  if (previousPort === undefined) delete process.env.PORT;
  else process.env.PORT = previousPort;
  if (server && !server.listening) await once(server, "listening");
  const testUrl = BASE_URL || `http://127.0.0.1:${server.address().port}${(process.env.BASE_URL || "/").replace(/\/?$/, "/")}`;
  const origin = new URL(testUrl).origin;
  console.log(`\n🔍 Running Website Regression Tests against: ${testUrl}\n`);

  try {
    for (const { name, type, launchOptions } of BROWSERS) {
      console.log(`\n${name}:`);
      let browser;
      try {
        browser = await type.launch({ headless: true, ...launchOptions });
      } catch (error) {
        if (name === "WebKit" && process.env.REQUIRE_WEBKIT !== "1" &&
            /Host system is missing dependencies to run browsers/.test(error.message)) {
          console.warn("  WebKit skipped: this host is missing required browser libraries.");
          continue;
        }
        throw error;
      }
      try {
        await checkSite(browser, testUrl, origin);
      } finally {
        await browser.close();
      }
    }
  } finally {
    if (server) await new Promise((resolve, reject) => server.close((error) => error ? reject(error) : resolve()));
  }
}

async function checkSite(browser, testUrl, origin) {
  const context = await browser.newContext();
  const page = await context.newPage();

  // Abort external resources that can hang or fail when offline / firewall restricted
  await page.route("**/*", (route) => {
    const url = route.request().url();
    if (new URL(url).origin !== origin) {
      route.abort();
    } else {
      route.continue();
    }
  });

  let totalTests = 0;
  let passedTests = 0;
  const failures = [];
  const consoleErrors = [];
  page.on("pageerror", (error) => failures.push(`Uncaught browser error: ${error.message}`));
  page.on("response", (response) => {
    if (new URL(response.url()).origin === origin && response.status() >= 400) {
      failures.push(`${response.status()} ${response.url()}`);
    }
  });
  page.on("console", (message) => {
    if (message.type() === "error" && message.location().url.startsWith(origin)) {
      consoleErrors.push(Promise.resolve(message.args()[1]?.evaluate((error) => error?.message || String(error)))
        .catch(() => "unavailable")
        .then((detail) => failures.push(`Browser console error: ${message.text()}${detail ? ` ${detail}` : ""}`)));
    }
  });

  for (const { path: route, hydrated } of ROUTES) {
    const url = new URL(route.slice(1), `${testUrl.replace(/\/?$/, "/")}`).href;
    const response = await page.goto(url, { waitUntil: "commit", timeout: 15000 });
    if (!response || !response.ok()) {
      throw new Error(`${route} returned ${response?.status() ?? "no response"}`);
    }
    if (hydrated) {
      await page.waitForFunction(() => document.documentElement.getAttribute("data-has-hydrated") === "true", null, { timeout: 15000 });
      await page.waitForSelector("#footer-custom-content #recode50footer", { state: "attached", timeout: 15000 });
    } else {
      console.log(`  ✓ ${route} returned ${response.status()}\n`);
      continue;
    }
    // Brief settle for hydration
    await page.waitForTimeout(600);

    for (const vp of VIEWPORTS) {
      totalTests++;
      await page.setViewportSize({ width: vp.width, height: vp.height });
      await page.waitForTimeout(150);

      const metrics = await page.evaluate(() => {
        const cw = document.documentElement.clientWidth;
        const sw = document.documentElement.scrollWidth;
        const diff = sw - cw;

        let culprits = [];
        if (diff > 1) {
          const allElements = document.querySelectorAll("*");
          for (const el of allElements) {
            const rect = el.getBoundingClientRect();
            if (rect.right > cw + 1) {
              const tag = el.tagName.toLowerCase();
              const cls = typeof el.className === "string" ? el.className.split(" ")[0] : "";
              culprits.push(`${tag}${cls ? "." + cls : ""} (right: ${Math.round(rect.right)}px, w: ${Math.round(rect.width)}px)`);
              if (culprits.length >= 3) break;
            }
          }
        }

        return { cw, sw, diff, culprits };
      });

      if (metrics.diff <= 1) {
        passedTests++;
        console.log(`  ✓ [${vp.name} - ${vp.width}x${vp.height}] ${route} (cw: ${metrics.cw}px, sw: ${metrics.sw}px)`);
      } else {
        failures.push({
          route,
          viewport: vp,
          metrics,
        });
        console.error(`  ✗ [${vp.name} - ${vp.width}x${vp.height}] ${route} OVERFLOW: scrollWidth ${metrics.sw}px > clientWidth ${metrics.cw}px (+${metrics.diff}px)`);
        if (metrics.culprits.length > 0) {
          console.error(`    Culprit elements: ${metrics.culprits.join(", ")}`);
        }
      }
    }
    if (route === "/models/") {
      const detailUrl = new URL("models/act-fp16-ov/", `${testUrl.replace(/\/?$/, "/")}`);
      await page.evaluate((path) => {
        history.pushState({}, "", path);
        dispatchEvent(new PopStateEvent("popstate"));
      }, detailUrl.pathname);
      await page.waitForFunction(() => document.title.includes("act-fp16-ov"), null, { timeout: 15000 });
      // Client navigation remounts the footer; the previous page's global loaded flag stays true.
      await page.waitForSelector("#footer-custom-content #recode50footer", { state: "attached", timeout: 15000 });
      console.log("  ✓ /models/act-fp16-ov/ resolved through client navigation");
    }
    console.log("");
  }

  await Promise.all(consoleErrors);
  console.log("--------------------------------------------------------------------------------");
  console.log(`Total Checks: ${totalTests} | Passed: ${passedTests} | Failed: ${failures.length}`);
  console.log("--------------------------------------------------------------------------------\n");

  if (failures.length > 0) {
    console.error(`❌ Website regression test failed: ${failures.length} error(s).`);
    for (const failure of failures) {
      if (typeof failure === "string") console.error(`  ${failure}`);
    }
    process.exitCode = 1;
  } else {
    console.log("✅ Website regression checks passed!");
  }
}

run().catch((err) => {
  console.error("Test runner error:", err);
  process.exit(1);
});
