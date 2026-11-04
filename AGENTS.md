# AGENTS.md

A README for coding agents (and a quick start for contributors) working on the
**robotics-ai-suite** website — the public developer site at
**https://developer.robotics.intel.com**.

This file is for *anyone* contributing to the site: human or agent, Intel or
community. It tells you how to set up, build, test, and ship a change on your own.
If you only need to touch the robotics source in `src/`, this guide doesn't apply —
follow the conventions in that part of the tree instead.

## Project overview

The site is built from two static generators that are stitched together and
deployed as one:

- **Docusaurus app** in `docs/website/` — the landing pages, React components, the
  models catalogue, navbar, site search, and client modules. Served at the site root.
- **Sphinx user-guide** in `docs/user-guide/` — the long-form documentation. Served under
  `/development-stack/`.

A change usually lives in one of these two; figure out which before you start.

## Dev environment

Prerequisites: **Node.js 22.x** (CI uses 22.12) and **Python 3.12+** (for the
Sphinx docs). Work from the `docs/website/` directory for anything Docusaurus:

```bash
cd docs/website
npm install          # first time; CI uses `npm ci`
npm start            # dev server with hot reload at http://localhost:3000/
```

To work on the Sphinx docs, build them so the Docusaurus dev server can stage the
HTML (the `/development-stack/` section is empty until you do):

```bash
cd docs
make build           # builds the Sphinx site into docs/out/
```

Then (re)start `npm start` so the staged docs are picked up.

> Platform note: the production build (`npm run build`) and some tooling assume a
> Unix-like environment, matching CI. On Windows, prefer the dev server
> (`npm start`) for local preview. Contributor-specific build quirks and
> workarounds are kept out of this file — capture them in your own notes.

## Repository structure

```
docs/
  Makefile           # build / lint / clean targets
  user-guide/        # Sphinx user-guide sources and assets
    _static/
    _templates/
    conf.py
  website/           # Docusaurus app (landing pages, models catalogue, search)
    src/             # React components, pages, theme swizzles, client modules
    static/          # static assets served at the site root
    plugins/         # build-time plugins (Sphinx staging, site search, models)
    docusaurus.config.js
.github/workflows/   # CI: deploy.yaml builds PR previews + production
```

## Build, preview, and test

From `docs/website/`:

- `npm start` — dev server with hot reload (primary local preview).
- `npm run build` — production build (runs `make build` in `docs/` first, then
  `docusaurus build`). Matches CI.
- `npm run serve` — serve a completed production build locally.
- `npm run test:mobile` — mobile viewport checks.

**Verify before you call it done:** build or preview the generator you touched and
look at the actual rendered page. Run the relevant check above and fix failures
before opening a PR.

## Content and code conventions

- **Match the surrounding code and prose.** Mimic the existing style, structure,
  and patterns rather than introducing your own.
- **Don't invent technical claims.** APIs, benchmarks, hardware specs, and
  behavior must come from a verifiable source. If you can't verify it, flag it and
  ask a maintainer or the relevant engineer — don't guess or paraphrase into fact.
- **Keep changes surgical.** Touch only what the task needs; don't refactor or
  reformat adjacent, unrelated content.
- **Links and navigation:** when you move or rename a page, update inbound links
  and the relevant nav/toctree. Broken internal doc links are not always caught by
  the build — check manually.
- **Comments and prose** should add context a reader can't get from the code or
  page itself; don't narrate change history.

## Compliance and privacy (please read)

The site runs on `*.intel.com` and must respect user consent:

- **Never ship analytics, tracking, or cookies that aren't consent-gated**, and
  never weaken the existing cookie-consent flow. Any change to analytics,
  cookies, or data collection must be flagged for maintainer review.

## Submitting a change (PRs)

1. Create a branch and make your change there.
2. Build/preview the affected generator and run the relevant check(s).
3. Open a pull request with a concise summary of what changed and why.
4. CI automatically builds and deploys a **preview** for your PR and comments the
   link on the PR — share it in your description and use it for review.
5. Address review feedback; a maintainer merges when it's ready.

Keep PRs focused and reviewable. Don't push directly to `main` — it is the
production branch, and merging to it publishes to the live site.

## Review and publishing

Merging to `main` publishes to the live production site via CI
(`.github/workflows/deploy.yaml`). To keep the published site accurate and
compliant:

- **Maintainers review and merge.** Don't merge your own PR to `main` without
  maintainer approval.
- **Technical content needs a verifiable source.** APIs, benchmarks, specs, and
  behavior must be confirmed with the people who own that content — don't publish
  unverified claims.
- **Flag externally visible changes.** If a change affects public messaging,
  branding, positioning, or launch timing, call it out in the PR for extra review
  rather than shipping it silently.

## Boundaries

- **Ask first** before: significant UX / information-architecture changes,
  anything touching analytics/consent/compliance, adding dependencies with broad
  impact, or anything that changes public messaging or branding.
- **Never**: merge to `main` or publish without maintainer sign-off; assert
  unverified technical content; ship non-consent-gated tracking; or run
  destructive / shared-infrastructure actions (force-push, history rewrites,
  deployment or hosting changes, deleting content or branches) without explicit
  confirmation.
