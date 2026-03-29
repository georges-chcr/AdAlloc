-- Visual: Budget breakdown by channel (selected vs excluded)
SELECT
    c.id                          AS channel_id,
    c.name                        AS channel_name,
    c.category,
    c.cost_eur,
    ar.selected,
    CASE ar.selected WHEN 1 THEN 'Selected' ELSE 'Excluded' END AS selection_status,
    ar.budget_used_eur,
    s.label                       AS scenario_label,
    s.id                          AS scenario_id
FROM allocation_results ar
JOIN channels  c ON c.id = ar.channel_id
JOIN scenarios s ON s.id = ar.scenario_id
ORDER BY s.id, ar.selected DESC, c.cost_eur DESC;


-- Visual: ROI optimized vs naive (sum of roi_obtained where selected=1 vs selected=0)
SELECT
    s.id                                                        AS scenario_id,
    s.label                                                     AS scenario_label,
    SUM(CASE ar.selected WHEN 1 THEN ar.roi_obtained ELSE 0 END) AS roi_optimized,
    SUM(CASE ar.selected WHEN 0 THEN ar.roi_obtained ELSE 0 END) AS roi_excluded,
    SUM(ar.roi_obtained)                                        AS roi_total
FROM allocation_results ar
JOIN scenarios s ON s.id = ar.scenario_id
GROUP BY s.id, s.label
ORDER BY s.id;


-- Visual: Channel selection rate across scenarios (% of scenarios where each channel was selected)
SELECT
    c.id                                                        AS channel_id,
    c.name                                                      AS channel_name,
    c.category,
    COUNT(DISTINCT ar.scenario_id)                              AS scenarios_evaluated,
    SUM(ar.selected)                                            AS times_selected,
    ROUND(
        100.0 * SUM(ar.selected) / COUNT(DISTINCT ar.scenario_id),
        2
    )                                                           AS selection_rate_pct
FROM allocation_results ar
JOIN channels c ON c.id = ar.channel_id
GROUP BY c.id, c.name, c.category
ORDER BY selection_rate_pct DESC;


-- Visual: Residual budget per scenario (total_budget_eur minus sum of budget_used_eur where selected=1)
SELECT
    s.id                                                        AS scenario_id,
    s.label                                                     AS scenario_label,
    s.total_budget_eur,
    SUM(CASE ar.selected WHEN 1 THEN ar.budget_used_eur ELSE 0 END) AS budget_allocated_eur,
    s.total_budget_eur
        - SUM(CASE ar.selected WHEN 1 THEN ar.budget_used_eur ELSE 0 END) AS residual_budget_eur,
    ROUND(
        100.0 * SUM(CASE ar.selected WHEN 1 THEN ar.budget_used_eur ELSE 0 END)
              / s.total_budget_eur,
        2
    )                                                           AS budget_utilization_pct
FROM allocation_results ar
JOIN scenarios s ON s.id = ar.scenario_id
GROUP BY s.id, s.label, s.total_budget_eur
ORDER BY s.id;
